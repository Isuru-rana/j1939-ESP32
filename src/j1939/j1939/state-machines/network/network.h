/**
 *
 * References:
 *
 * 1. J1939-81 (draft MAY2003)
 * 2. J1939-21 (DEC2006)
 * 3. AddressResolution.md v0.1
 */
#pragma once

#include "base.h"

namespace embr { namespace j1939 { namespace sm { inline namespace v1 {


template <ESTD_CPP_CONCEPT(internal::concepts::AddressManager) AddressManager,
    class TimePoint>
struct network : network_base
{
#ifdef ESP_PLATFORM
    static constexpr const char* TAG = "sm::network";
#endif

    using base_type = network_base;
    using address_manager_type = AddressManager;
    using time_point = TimePoint;

    using network_base::process_incoming;

    // DEBT: Do some data hiding

    address_manager_type address_manager_;

    // Depending on whether we're claiming or request for claim we'll
    // next_event_ 250ms or 1250ms.  Also expected but not yet implemented
    // is a pre-send next_event_ with bus_collision_delay
    // NOTE: We miss old 'last_claim' but this is more efficient
    time_point next_event_;

    constexpr time_point next_event() const { return next_event_; }

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

    bool contended();


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

    // DEBT: Quick and dirty adaptation from non-state-machine variety.  Likely needs
    // cleanup in context of state machine design
    // Returns true when 'wake' is updated.  So far no scenarios exist where multiple
    // schedule_claiming calls are needed per 'wake'
    template <class Transport>
    bool scheduled_claiming(Transport& t, time_point* wake, time_point current);

    template <class Transport>
    void start(Transport& transport, time_point current);

    ///
    /// @param wake
    /// @param current
    /// @return true on schedule requested (wake updated), false on "done"
    /// @remark claimed/claiming state is REQUIRED
    bool update_state_after_send_claim(time_point* wake, time_point current);

    ///
    /// @tparam Transport
    /// @param wake EXPERIMENTAL - 1:1 with next_event_ at the moment
    /// @param current
    /// @param do_schedule
    /// @return
    template <class Transport>
    bool process_incoming_internal(Transport&, const pdu<pgns::address_claimed>&,
        time_point* wake,
        time_point current,
        bool* do_schedule);

    template <class Transport>
    bool process_incoming(Transport& t, const pdu<pgns::address_claimed>& p,
        const context<TimePoint>& c)
    {
        bool do_schedule = false;
#if FEATURE_EMBR_J1939_TP_CONTEXT_NEXT
        time_point* wake = c.next_;
#else
        time_point dummy;
        time_point* wake = &dummy;
#endif

        bool r = process_incoming_internal(t, p, wake, c.current, &do_schedule);

        // EXPERIMENTAL
#if FEATURE_EMBR_J1939_TP_CONTEXT_NEXT
        if(do_schedule)
            *c.next_ = next_event_;
#endif

        return r;
    }


#if FEATURE_EMBR_J1939_TP_CONTEXT_NEXT
    template <class Transport>
    bool process_outgoing(Transport& t, const context<TimePoint>& c)
    {
        //if(substate_ != substates::sending) return false;

        if(c.current < next_event_) return false;

        //return process_outgoing_internal(t, c);
        scheduled_claiming(t, c.next_, c.current);

        // DEBT: process_outgoing returns a bool indicating whether further immediate processing is
        // expected.  scheduled_claiming returns a bool indicating whether a future event should be
        // scheduled.  At present, it NEVER requires further immediate processing

        return false;
    }
#endif

};

}}}}
