#pragma once

#include "parser.h"

namespace embr { namespace can { namespace slcan { inline namespace v0 {

template <ESTD_CPP_CONCEPT(concepts::Impl) Impl>
const char* parser<Impl>::parse(estd::string_view s)
{
    char c = s[0];
    const bool sz1 = s.size() == 1;

    estd::string_view param = s.substr(1);

    switch(c)
    {
        case 'S':       // setup bitrate
            return bitrate(param);

        case 's':       // setup BTR0/BTR1 style
            return ERROR;   // Not supported

        case 'O':       // open CAN channel
            if(!sz1) return ERROR;
            return impl().open(false);

        case 'L':       // open CAN channel (listen only)
            if(!sz1) return ERROR;
            return impl().open(true);

        case 'C':       // close CAN channel
            if(!sz1) return ERROR;
            return impl().close();

        case 'F':       // Read status flags
            return alerts();

        case 'A':       // Poll all (deprecated)
        {
            if(!sz1) return ERROR;

            char* s2 = get_frame_to_send_to_host(to_host_buffer);
            *s2 = 0;

            // DEBT: Should return multiples.  Better served by reworking
            // serialization code to use an ostream/ostreambuf directly
            return s2;
        }

        case 'N':
            return "NE000\r";

        case 'P':       // Poll single (deprecated)
        {
            if(!sz1) return ERROR;

            char* s2 = get_frame_to_send_to_host(to_host_buffer);
            *s2 = 0;

            return s2;
        }

        case 'r':       // Transmit 11bit frame (RTR)
            return transmit(param, false, true);

        case 'R':       // Transmit 29bit frame (RTR)
            return transmit(param, true, true);

        case 't':       // Transmit 11bit frame
            return transmit(param, false, false);

        case 'T':       // Transmit 29bit frame
            return transmit(param, true, false);

        case 'U':       // USB UART supports any baudrate
            return OK;

        case 'V':       // get version number'
            return "V0001\r";

        case 'X':       // Auto Poll/Send ON/OFF
            return pollmode(param);

        case 'W':
            break;

        default: break;
    }

    return ERROR;
}

}}}}