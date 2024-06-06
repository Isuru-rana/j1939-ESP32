#pragma once

#include "../data_field/network.hpp"

#include "tp/base.h"

// v1 here while tp still v0 because we (plan to have) adapted in clunky but functional network_ca_base
namespace embr { namespace j1939 { namespace sm { inline namespace v1 {

struct ca_base
{
    template <pgns pgn>
    using pdu = pdu<pgn>;
};

struct network_base : ca_base //,
    //embr::Service   // Ready and waiting, premature to start migrating to this atm
{
#ifdef ESP_PLATFORM
    static constexpr const char* TAG = "network_ca";
#endif

    template <class TimePoint>
    using context = sm::tp::v0::context<TimePoint>;

    // DEBT: Upgrade this to embr 'service' architecture
    enum class states
    {
        unstarted,
        requesting,         ///< Request for address_claimed [2] A5, A6, A7 Initialize
        claiming,           ///< Address Claim - emit and wait
        claimed,            ///< Address Claim success without contention
        claim_failed
    };


    // Guidance from [1] Appendix D: State Transition Diagrams
    enum class substates
    {
        unstarted,

        // generic reused states
        sending,            ///< Indicate transport is in process of emitting something (see 'states' for what)
        waiting,            ///< Indicate main state has done what it can, and now waiting for response traffic

        // requesting state

        /// Waiting period (1250ms) after we emit a request for address claim
        request_waiting, // = waiting,  // DEBT: scheduled_claiming doesn't quite disambiguate enough here

        // claiming state

        /// Waiting period (250ms) after we emit a claim address
        claim_waiting,  // = waiting,
        contending,         ///< Evaluation period after we receive a contending address
        claim_send_error,   ///< Same as 'waiting' but bus/send error occurred [3] 1.1.4
        reclaim_waiting,    ///< Waiting period of 0-153ms preceding re-transmit of claim [1] 4.4.4.3
        bus_off,            ///< FIX: These bus_off states appear to be in conflict with reclaim_waiting
        bus_off_recover,

        // claimed state
        expired,            ///< Claim waiting period expired with no incident, meaning we succeeded

        // claim failed state
        failed,             ///< Root substate when we've given up trying to get SA
        cannot_claim_waiting,   ///< Waiting period of 0-153ms preceding the emit of "cannot claim" [1] 4.2.2.3
    };

    states state = states::unstarted;
    substates substate = substates::unstarted;

    using address_traits = spn::internal::address_type_traits_base;
    using address_type = estd::layer1::optional<uint8_t, address_traits::null>;

protected:
    // TODO: Optimize to use sparse/layer0/layer2 name but not at the exclusion
    // of the edge case where a NAME can be totally changed
    layer1::NAME name_{null_t{}};

    // TODO: Change to non-optional (since state machine handles that)
    // May be claimed, claiming or cannot claim depending on
    // state machine
    address_type address_;

#if UNIT_TESTING
public:
#endif
    template <class Container>
    constexpr explicit network_base(const NAME<Container>& name) :
        name_{name}
    {}

    template <class Layer0Name, estd::enable_if_t<estd::is_base_of<
        embr::j1939::layer0::sparse_tag, Layer0Name>::value, bool> = true>
    explicit network_base(Layer0Name sparse)
    {
        sparse.populate(name_);
    }

    // [1] Figure D1
    constexpr bool skip_timeout() const
    {
        return (address_ >= 0 && address_ <= 127) ||
               (address_ >= 248 && address_ <= 253);
    }


public:
    const address_type& address() const { return address_; }

    // DEBT: Would like this to work, though perhaps not specifically
    // preferred.  See layer2::NAME in fwd for more details as to
    // why it doesn't work yet
    //const layer2::NAME name() { return { name_.data() }; }
    const layer1::NAME& name() { return name_; }

    using milliseconds = estd::chrono::milliseconds;

    static constexpr milliseconds cannot_claim_address_max_timeout() { return milliseconds(153); }
    // [1.5] 4.4.4.3
    static constexpr milliseconds bus_collision_delay() { return milliseconds(153); }

    static constexpr milliseconds address_claim_timeout() { return milliseconds(250); }
    static constexpr milliseconds request_for_address_claim_timeout()
    {
        return milliseconds(1250);
    }

    constexpr bool arbitrary_address_capable() const
    {
        return name_.arbitrary_address_capable();
    }

    template <class Transport>
    void send_claim(Transport& t, pdu<pgns::address_claimed>& p, uint8_t sa);

    template <class Transport>
    void send_claim(Transport& t)
    {
        pdu<pgns::address_claimed> p{null_t{}};

        send_claim(t, p, *address_);
    }

    // [1] 4.2.1
    template <class Transport>
    void send_request_for_address_claimed(Transport&, uint8_t da);


    template <class Transport>
    void send_cannot_claim(Transport& t, pdu<pgns::address_claimed>& p)
    {
        p.can_id().source_address(address_traits::null);
        p.payload() = name_;
        transport_traits<Transport>::send(t, p);

    }

    template <class Transport>
    void send_cannot_claim(Transport& t)
    {
        pdu<pgns::address_claimed> p{null_t{}};

        p.can_id().destination_address(address_traits::global);

        send_cannot_claim(t, p);
    }


    void start();

    constexpr bool has_address() const
    {
        return address_.has_value();
    }

    /// Is the address in this claimed message the same as the one we intend to use?
    constexpr bool is_contender(const pdu<pgns::address_claimed>& p) const
    {
        return p.source_address() == address_;
    }

    // DEBT: Need to coordinate this better with 'timeout' assignment,
    // otherwise we'll definitely run into a form of jitter
    // DEBT: Need better name, more along the lines of "next claim timeout"
    template <class Rep, class Period>
    bool schedule_address_claim_timeout(estd::chrono::duration<Rep, Period>* wake)
    {
        if(skip_timeout()) return false;

        *wake += address_claim_timeout();
        return true;
    }

    template <class Transport, class TimePoint>
    bool process_outgoing(Transport&, const context<TimePoint>&);
};


template <ESTD_CPP_CONCEPT(internal::concepts::AddressManager) AddressManager,
    class TimePoint>
struct network : network_base
{
    using base_type = network_base;
    using address_manager_type = AddressManager;
    using time_point = TimePoint;

    address_manager_type address_manager_;

    address_type find_new_address()
    {
        if(address_manager().depleted())
            return {};
        else
            return address_manager().get_candidate();
    }

    void track(const pdu<pgns::address_claimed>&)
    {
        // TODO: Map incoming CA/SA/NAMEs, probably via some kind of impl associated with
        // SA generation
    }


    milliseconds get_send_claim_defer();

    /// In response to a contending incoming address claim, initiate process of
    /// coming up with a new candidate SA
    template <class Transport>
    void evaluate_contender(Transport& t, const pdu<pgns::address_claimed>& p)
    {
        // TODO: We'll need to emit our own address claimed, cannot claim and
        // perhaps do some requests to see what address we should try for

        //state = states::claiming;
        //substate = substates::contending;
    }

    // DEBT: Poor naming.  State machine assist to react to incoming address claim
    // which may contend
    template <class Transport>
    bool evaluate_contenders(Transport& t, const pdu<pgns::address_claimed>& p);


    address_manager_type& address_manager() { return address_manager_; }

    template <class ...Args>
    constexpr explicit network(address_manager_type& am, Args&&...args) :
        base_type(std::forward<Args>(args)...),
        address_manager_{am}
    {}

    template <class ...Args>
    constexpr explicit network(address_manager_type&& am, Args&&...args) :
        base_type(std::forward<Args>(args)...),
        address_manager_{std::move(am)}
    {}
};

}}}}
