#pragma once

#include <estd/charconv.h>
#include <estd/locale.h>
#include <estd/string_view.h>

#include <can/reference.h>
#include <can/loopback.h>

#if __cpp_lib_concepts
#include <concepts>
#endif

namespace embr { namespace can { namespace slcan { inline namespace v0 {

#if __cpp_lib_concepts
namespace concepts {

template <class T>
concept Impl = requires(T t)
{
    T::bitrates_;

    t.open();
    t.close();
};

}
#endif

namespace impl {

// Transport abstraction is very hard.  Do up impl pattern for some auxiliary
// transport specifics rather than a full on transport abstraction

struct base
{
    static constexpr unsigned bitrates_[] =
        { 10, 20, 50, 100, 125, 250, 500, 800, 1000 };
};

struct loopback : base
{
    using transport_type = can::loopback_transport;

    transport_type transport_;

    transport_type& transport() { return transport_; }

    const char* open()
    {
        return "\r";
    }

    const char* close()
    {
        return "\r";
    }

    const char*  bitrate(unsigned v)
    {
        return "\r";
    }
};

}

template <ESTD_CPP_CONCEPT(concepts::Impl) Impl = impl::loopback>
class parser : impl::base   // DEBT
{
    Impl impl_;

public:
    using transport_type = typename Impl::transport_type;
    using frame_type = typename transport_type::frame;
    using frame_traits = can::frame_traits<frame_type>;

    static constexpr const char* OK = "\r";
    static constexpr const char* ERROR = "\7";
    static constexpr const char* OK_NEW = "z\r";

    using view = estd::string_view;

#if UNIT_TESTING
public:
#else
protected:
#endif

    Impl& impl() { return impl_; }

    // Turn ASCII representation into native frame
    estd::errc deserialize(view in, frame_type* out, bool extended)
    {
        uint32_t v;
        unsigned bump = extended ? 8 : 4;
        const char* current = in.begin();
        //const char* const end = in.end();

        estd::from_chars_result r = estd::from_chars(current, current + bump, v, 16);

        frame_traits::id(*out, v);

        current += bump;

        r = estd::from_chars(current, current + 1, v, 16);

        if(r.ec != 0) return r.ec;

        ++current;

        frame_traits::length(*out, v);

        uint8_t* payload = frame_traits::payload(*out);

        while(v--)
        {
            uint8_t v2;

            r = estd::from_chars(current, current + 2, v2, 16);

            if(r.ec != 0) return r.ec;

            current += 2;

            *payload++ = v2;
        }

        return estd::errc{0};
    }

    void serialize(const frame_type& in, char* out)
    {
        // Holding off until https://github.com/malachi-iot/estdlib/issues/42 so that we don't
        // reinvent formatting/padding code
        //estd::to_chars_result r = estd::to_chars(out, out + 8, frame_traits::id(in), 16);

        //r.
    }

    const char* transmit(view v, bool extended, bool rtr)
    {
        // Not supported yet
        if(rtr) return  ERROR;

        frame_type frame;

        deserialize(v, &frame, extended);

        return impl().transport().send(frame) ? OK : ERROR;
    }

    const char* bitrate(view s)
    {
        if(s.size() != 1) return ERROR;

        unsigned v = s[0] - '0';
        if(v > 8) return ERROR;

        return impl().bitrate(bitrates_[v]);
    }

public:
    const char* parse(estd::string_view s)
    {
        char c = s[0];
        const bool sz1 = s.size() == 1;

        estd::string_view param = s.substr(1);

        switch(c)
        {
            case 'S':       // setup bitrate
                return bitrate(param);

            case 's':       // setup BTR0/BTR1 style
                break;

            case 'O':       // open CAN channel
                if(!sz1) return ERROR;
                return impl().open();

            case 'L':       // open CAN channel (listen only)
                break;

            case 'C':       // close CAN channel
                if(!sz1) return ERROR;
                return impl().close();

            case 'r':       // Transmit 11bit frame (RTR)
                return transmit(param, false, true);

            case 'R':       // Transmit 29bit frame (RTR)
                return transmit(param, true, true);

            case 't':       // Transmit 11bit frame
                return transmit(param, false, false);

            case 'T':       // Transmit 29bit frame
                return transmit(param, true, false);

            case 'V':       // get version number'
                break;

            case 'X':       // Auto Poll/Send ON/OFF
                break;

            default: break;
        }

        return ERROR;
    }
};

}}}}
