#pragma once

#include "parser.h"

namespace embr { namespace can { namespace slcan { inline namespace v0 {

template <ESTD_CPP_CONCEPT(concepts::Impl) Impl>
template <class S, class B>
auto parser<Impl>::parse(estd::string_view in, ostream<S, B>& out) -> ostream<S, B>&
{
    char c = in[0];
    const bool sz1 = in.size() == 1;

    estd::string_view param = in.substr(1);
    //const char* ret;

    switch(c)
    {
        case 'S':       // setup bitrate
            return out << bitrate(param);

        case 's':       // setup BTR0/BTR1 style
            break;   // Not supported

        case 'O':       // open CAN channel
            if(!sz1 || impl().opened()) return out << ERROR;
            return out << impl().open(false);

        case 'L':       // open CAN channel (listen only)
            if(!sz1 || impl().opened()) return out << ERROR;
            return out << impl().open(true);

        case 'C':       // close CAN channel
            if(!sz1 || !impl().opened()) return out << ERROR;
            return out << impl().close();

        case 'F':       // Read status flags
            return alerts(out);

        case 'A':       // Poll all (deprecated)
        {
            if(!sz1 || autopoll_) return out <<  ERROR;

            return send_frame_to_host(out);
            //*s2 = 0;

            // DEBT: Should return multiples.  Better served by reworking
            // serialization code to use an ostream/ostreambuf directly
            //return s2;
        }

        case 'N':
            return out << "NE000\r";

        case 'P':       // Poll single (deprecated)
        {
            if(!sz1 || autopoll_) return out << ERROR;

            return send_frame_to_host(out);
        }

        case 'r':       // Transmit 11bit frame (RTR)
            return out << transmit(param, false, true);

        case 'R':       // Transmit 29bit frame (RTR)
            return out << transmit(param, true, true);

        case 't':       // Transmit 11bit frame
            return out << transmit(param, false, false);

        case 'T':       // Transmit 29bit frame
            return out << transmit(param, true, false);

        case 'U':       // USB UART supports any baudrate
            return out << OK;

        case 'V':       // get version number
            return out << "V0001\r";

        case 'X':       // Auto Poll/Send ON/OFF
            return out << pollmode(param);

        case 'W':
            break;

        case 'Z':
            if(!sz1) return out << ERROR;
            switch(param[0])
            {
                case '0':   timestamps_ = false; return out << OK;
                case '1':   timestamps_ = true; return out << OK;
                default: break;
            }
            break;

        default: break;
    }

    return out << ERROR;
}


template <ESTD_CPP_CONCEPT(concepts::Impl) Impl>
template <class S, class B>
auto parser<Impl>::serialize(const frame_type& in, ostream<S, B>& out) -> ostream<S, B>&
{
    const bool extended = frame_traits::extended(in);

    // DEBT: Minor debt, consider using the nifty 0x20 mask to convert to lower case

    if(frame_traits::rtr(in))
        out << (extended ? 'R' : 'r');
    else
        out << (extended ? 'T' : 't');

    out.setf(estd::ios_base::hex | estd::ios_base::uppercase,
        estd::ios_base::basefield);
    out.width(extended ? 8 : 4);
    out.fill('0');

    out << frame_traits::id(in);

    unsigned length = frame_traits::length(in);

    out.put('0' + length);

    out.width(2);

    const uint8_t* payload = frame_traits::payload(in);

    while(length--) out << *payload++;

    if(timestamps_)
    {
        out.width(4);
        out << impl().timestamp_ms();
    }

    return out << OK;
}


}}}}