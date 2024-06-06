/**
 *
 * References:
 *
 * 1. J1939-81 (draft MAY2003)
 * 2. J1939-21 (DEC2006)
 * 3. AddressResolution.md v0.1
 */
#pragma once

#include <estd/chrono.h>
#include <estd/functional.h>
#include <estd/optional.h>
#include <estd/type_traits.h>

#include <embr/service.h>

#include "../ca.h"
#include "../data_field/transport_protocol.hpp"

#include "internal/rng_address_manager.h"

#include "../state-machines/tp/base.h"
#include "../state-machines/network.h"

#include "fwd.h"

namespace embr { namespace j1939 {

namespace impl {

namespace experimental {

// FIX: This is all sloppy, just doing this to bring things online.  We may instead consider always
// demanding chrono for CA scheduler impl time_point

template <class TSchedulerImpl, class TTimePoint = typename TSchedulerImpl::time_point>
struct ca_time_helper;

template <class TSchedulerImpl>
struct ca_time_helper<TSchedulerImpl, unsigned>
{
    static constexpr unsigned milliseconds(unsigned ms) { return ms; }
};

template <class TSchedulerImpl, class Rep, class Period>
struct ca_time_helper<TSchedulerImpl, estd::chrono::duration<Rep, Period> >
{
    typedef estd::chrono::duration<Rep, Period> time_point;

    static constexpr time_point milliseconds(Rep ms)
    {
        return estd::chrono::milliseconds(ms);
    }
};

}



// Pertains to [1] 5.10
template <class Transport, class Scheduler,
    ESTD_CPP_CONCEPT(internal::concepts::AddressManager) AddressManager>
struct network_ca : impl::controller_application<Transport>,
        sm::network<AddressManager, typename Scheduler::time_point>
{
    typedef j1939::impl::controller_application<Transport> base_type;
    using nca_base_type = sm::network<AddressManager, typename Scheduler::time_point>;

    using typename base_type::transport_type;
    using typename base_type::frame_type;
    using typename base_type::frame_traits;

    using typename nca_base_type::time_point;
    using typename nca_base_type::address_traits;
    using typename nca_base_type::address_type;
    using typename nca_base_type::states;
    using typename nca_base_type::substates;

    using nca_base_type::name_;
    using nca_base_type::address_;
    using nca_base_type::state;
    using nca_base_type::substate;
    using nca_base_type::address_manager;
    using nca_base_type::find_new_address;
    using nca_base_type::next_event_;

    typedef transport_traits<transport_type> _transport_traits;

    typedef Scheduler scheduler_type;

    // DEBT: Filter by scheduler_type::impl::is_chrono, that is a necessity
    // DEBT: Filter by scheduker_type::impl::tag::Function, that is a
    //       necessity at the moment, though network_ca could be specialized to
    //       operate in other modes such as 'Traditional'

    typedef AddressManager address_manager_type;

    scheduler_type& scheduler;

    // DEBT: Instead, expose impl_type directly from sechduler_type
    typedef estd::remove_reference_t<decltype(scheduler.impl())> scheduler_impl_type;
    //typedef typename scheduler_impl_type::time_point time_point;
    typedef typename scheduler_impl_type::function_type fn_test;

    using function_type = typename scheduler_impl_type::function_type;
    //typedef impl::experimental::ca_time_helper<scheduler_impl_type> helper;

    transport_type* t;

    void send_claim(transport_type& t)
    {
        nca_base_type::send_claim(t);

        // DEBT: May not want to do this IN emitter method itself
        next_event_ = scheduler.impl().now() + nca_base_type::address_claim_timeout();
    }

    // Emits address claim over transport and assures a followup of
    // is scheduled for 250ms later
    void send_claim_and_schedule(transport_type& t, pdu<pgns::address_claimed>& p, uint8_t sa);

    // Scheduler calls this guy
    void scheduled_claiming(time_point* wake, time_point current);

    void scheduled_cannot_claim(time_point* wake, time_point current)
    {
        switch(substate)
        {
            //case substates::
            default:
                break;
        }
    }

    void scheduled(time_point* wake, time_point current)
    {
        switch(state)
        {
            case states::claiming:
                scheduled_claiming(wake, current);
                break;

            case states::claim_failed:
                scheduled_cannot_claim(wake, current);
                break;

            default:
                break;
        }
    }

    struct wake_functor
    {
        network_ca& this_;

        void operator()(time_point* wake, time_point current)
        {
            this_.scheduled(wake, current);
        }
    };

    typename function_type::template model<wake_functor> wake_model{wake_functor{*this}};

    // EXPERIMENTAL
    template <class TLayer0Name>
    using init1 = embr::j1939::experimental::tuple_init<TLayer0Name, scheduler_type&>;

    // EXPERIMENTAL
    template <class TLayer0Name, estd::enable_if_t<estd::is_base_of<
        embr::j1939::layer0::sparse_tag, TLayer0Name>::value, bool> = true>
    static init1<TLayer0Name> get_init(TLayer0Name sparse, scheduler_type& scheduler)
    {
        // DEBT: For values at a minimum we shouldn't need to forward
        // here
        return init1<TLayer0Name>{ std::forward<TLayer0Name>(sparse), scheduler };
    }

    template <class TLayer0Name, estd::enable_if_t<estd::is_base_of<
        embr::j1939::layer0::sparse_tag, TLayer0Name>::value, bool> = true>
    explicit constexpr network_ca(TLayer0Name sparse,
        scheduler_type& scheduler) :
        nca_base_type(address_manager_type{}, sparse),
        scheduler{scheduler}
    {}

    // EXPERIMENTAL
    template <class TLayer0Name, estd::enable_if_t<estd::is_base_of<
        embr::j1939::layer0::sparse_tag, TLayer0Name>::value, bool> = true>
    explicit constexpr network_ca(init1<TLayer0Name> v) :
        network_ca(estd::get<0>(v), estd::get<1>(v))
    {}

    template <class TContainer>
    explicit constexpr network_ca(const NAME<TContainer>& name,
        scheduler_type& scheduler) :
        nca_base_type(address_manager_type{}, name),
        scheduler{scheduler}
        //f([&](time_point* wake, time_point current) { scheduled(wake, current); })
    {

    }

    template <class TContainer>
    explicit constexpr network_ca(const NAME<TContainer>& name,
        scheduler_type& scheduler,
        address_manager_type& am) :
        nca_base_type(am, name),
        scheduler{scheduler}
    {

    }

    // DEBT: I think we'd prefer to do this at constructor, but for now is easier
    // to do a manual start call
    void start(transport_type& t);

    template <pgns pgn>
    inline bool process_incoming(transport_type& t, pdu<pgn> p) { return false; }

    bool process_incoming(transport_type& t, const pdu<pgns::address_claimed>& p);

    bool process_request_for_address_claimed(transport_type& t, const pdu<pgns::request>& p);

    bool process_incoming(transport_type& t, const pdu<pgns::request>& p);
};


}

}}
