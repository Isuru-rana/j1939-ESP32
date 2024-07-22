#pragma once

#include <estd/iomanip.h>

// DEBT: Potentially prefer subject/observer or deeper impl participation to avoid platform specifity
// here, even in a diagnostic sense
#if ESP_PLATFORM
#include <esp_log.h>
#endif

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

        case 'f':       // Read extended status flags [2]
            return status(out);

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

        case 'Q':       // Auto-start config
            return out << impl().autostart(autostart_modes(param[0] - '0'));

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

    const uint8_t* payload = frame_traits::payload(in);

    while(length--) out << estd::setw(2) << *payload++;

    if(timestamps_)
    {
        out.width(4);
        out << impl().timestamp_ms();
    }

    return out << OK;
}

// Turn ASCII representation into native frame
template <ESTD_CPP_CONCEPT(concepts::Impl) Impl>
template <class CharIt>
estd::errc parser<Impl>::deserialize(CharIt in, frame_type* out, bool extended)
{
    //constexpr auto success = estd::errc{};

    uint32_t v;
    unsigned bump = extended ? 8 : 4;
    CharIt current = in;
    //const char* current = in.begin();
    //const char* const end = in.end();

    // TODO: Do ALERT_DATA_STREAM on result errors

    estd::from_chars_result r = estd::from_chars(current, current + bump, v, 16);

    // DEBT: See below ec comparison
    if(!(r.ec == 0)) return r.ec;

    frame_traits::id(*out, v);

    current += bump;

    r = estd::from_chars(current, current + 1, v, 16);

    // DEBT: See below ec comparison
    if(!(r.ec == 0)) return r.ec;

    ++current;

    frame_traits::length(*out, v);

    uint8_t* payload = frame_traits::payload(*out);

    while(v--)
    {
        uint8_t v2;

        r = estd::from_chars(current, current + 2, v2, 16);

        // DEBT: According to https://en.cppreference.com/w/cpp/utility/to_chars
        // the ideal version of this *might* be r.ec != estd::errc{}
        // DEBT: Also, our errc needs != operator in general
        if(!(r.ec == 0)) return r.ec;

        current += 2;

        *payload++ = v2;
    }

    return estd::errc{};
}

template <ESTD_CPP_CONCEPT(concepts::Impl) Impl>
const char* parser<Impl>::transmit(view v, bool extended, bool rtr)
{
    if(!impl().opened())    return ERROR;

    // Not supported yet, but almost
    if(rtr) return  ERROR;

    frame_type frame;

    frame_traits::rtr(frame, rtr);
    frame_traits::extended(frame, extended);

    estd::errc r = deserialize(v.begin(), &frame, extended);

#if ESP_PLATFORM
    static const char* TAG = "parser::transmit";

    // 08JUL24 - suspect side effects/pointer issues with frame payload.  Logging enabled during VERBOSE
    // raises chance of correct data received at console side.
    // TODO: Try this in DEBUG mode (VMware USB went offline again during testing)
    const uint8_t* payload = frame_traits::payload(frame);
    //const uint8_t* payload = frame.data;

    ESP_LOG_BUFFER_HEX_LEVEL(TAG,
        payload,
        frame_traits::length(frame),
        ESP_LOG_DEBUG);
#endif

    if(r == 0)
        return impl().transport().send(frame) ?
            (autopoll() ? OK_AUTOPOLL : OK) : ERROR;
    else
    {
        alerts_ |= ALERT_DATA_STREAM;
        return ERROR;
    }
}


}}}}
