#pragma once

#include <j1939/data_field/oel.hpp>
//#include <j1939/data_field/switch_bank.hpp>

//#include <j1939/ca.hpp>
#include <j1939/ca.h>
#include <j1939/cas/internal/fwd.h>

#include <j1939/pdu.h>

namespace test {

using namespace embr;
using namespace embr::j1939;

template <class TTransport>
struct SyntheticCA :
    j1939::impl::controller_application<TTransport>,
    cs::v1::base
{
    typedef j1939::impl::controller_application<TTransport> base_type;
    using typename base_type::transport_type;
    using typename base_type::frame_type;
    using frame_traits = can::frame_traits<frame_type>;
    using cs::v1::base::process_incoming;
    using typename cs::v1::base::result;

    typedef transport_traits<transport_type> _transport_traits;

    int switch_bank_control_counter = 0;
    int oel_counter = 0;

    result process_incoming(transport_type&, const pdu<pgns::switch_bank_control>&)
    {
        ++switch_bank_control_counter;
        return result::ok();
    }

    result process_incoming(transport_type& t, const pdu<pgns::oel>& p)
    {
        switch(p.turn_signal_switch())
        {
            case enum_type<spns::turn_signal_switch>::left_turn_to_be_flashing:
            {
                ++oel_counter;
                // DEBT: Not really a great command/response chain, but better than a pure echoback
                pdu<pgns::switch_bank_control> pdu_response{null_t{}};

                _transport_traits::send(t, pdu_response);
                break;
            }

            default:
                return result::ignore();
        }

        return result::ok();
    }
};

template <class TTransport>
struct SyntheticCA2 :
    j1939::impl::controller_application<TTransport>,
    cs::v1::base
{
    int unhandled_counter = 0;

    // Was experiencing inexplicable SIGTRAP here.  Turns out I forgot to
    // return a value and once again I was only warned (not error'd) about it
    result process_incoming_default(TTransport&,
        const typename TTransport::frame&)
    {
        ++unhandled_counter;
        return result::ignore();
    }
};

}
