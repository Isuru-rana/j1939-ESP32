#pragma once

#include <estd/charconv.h>
#include <estd/iterator.h>
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

    t.open(bool{});
    t.close();
};

}
#endif

namespace impl {

// Transport abstraction is very hard.  Do up impl pattern for some auxiliary
// transport specifics rather than a full on transport abstraction

struct base
{
    bool opened_ = false;

    bool opened() const { return opened_; }

    enum bitrates_enum
    {
        BITRATE_10K,
        BITRATE_20K,
        BITRATE_50K,
        BITRATE_100K,
        BITRATE_125K,
        BITRATE_250K,
        BITRATE_500K,
        BITRATE_800K,
        BITRATE_1000K
    };

    static constexpr unsigned bitrates_[] =
        { 10, 20, 50, 100, 125, 250, 500, 800, 1000 };
};

struct loopback : base
{
    using transport_type = can::loopback_transport;

    transport_type transport_;

    transport_type& transport() { return transport_; }

    const char* open(bool listen_only)
    {
        return "\r";
    }

    const char* close()
    {
        return "\r";
    }

    const char* bitrate(unsigned idx, unsigned rate)
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

    enum alerts : uint8_t
    {
        ALERT_RX_FIFO_FULL,
        ALERT_TX_FIFO_FULL,
        ALERT_ARBITRATION_LOST = 1 << 6,
        ALERT_BUS_ERROR = 1 << 7
    };

    static constexpr const char* OK = "\r";
    static constexpr const char* ERROR = "\7";
    static constexpr const char* OK_NEW = "z\r";

    // Approximately
    static constexpr const unsigned max_frame_str_size = 30;

    using view = estd::string_view;

#if UNIT_TESTING
public:
#else
protected:
#endif

    alerts alerts_ {};
    bool autopoll_ = false;

    Impl& impl() { return impl_; }

    // Primarily useful for polled mode, but also if to host USB doesn't keep up for some
    // reason, can be helpful too
    estd::layer1::queue<frame_type, 5> frames_to_send;

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
            if(!(r.ec == 0)) return r.ec;

            current += 2;

            *payload++ = v2;
        }

        return estd::errc{0};
    }

    template <class CharIter>
    CharIter serialize(const frame_type& in, CharIter out, bool extended)
    {
        // DEBT: Extract char_type from iter
        using num_put = estd::num_put<char, CharIter>;
        num_put np; // DEBT: It feels like one of these days he might end up requiring an instance.  Not today though
        estd::ios_base fmt;

        fmt.setf(estd::ios_base::hex | estd::ios_base::uppercase,
            estd::ios_base::basefield);
        fmt.width(extended ? 8 : 4);

        out = np.put(out, fmt, '0', frame_traits::id(in));

        unsigned length = frame_traits::length(in);

        *out++ = '0' + length;

        fmt.width(2);

        const uint8_t* payload = frame_traits::payload(in);

        while(length--)
        {
            out = np.put(out, fmt, '0', *payload++);
        }

        return out;
    }

    // for 'parse' to use as its response buffer
    char to_host_buffer[max_frame_str_size];

    template <class CharIt>
    CharIt get_frame_to_send_to_host(CharIt s)
    {
        if(!frames_to_send.empty())
        {
            // DEBT: Ascertain via frame_traits whether this is extended or not
            s = serialize(frames_to_send.front(), s, true);

            frames_to_send.pop();
        }

        *s++ = '\r';

        return s;
    }


    // received from CAN bus
    void on_receive(const frame_type& frame)
    {
        frames_to_send.push(frame);
    }

    // send out over CAN bus
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

        return impl().bitrate(v, bitrates_[v]);
    }

    const char* pollmode(view s)
    {
        if(s.size() != 1) return ERROR;

        const char c = s[0];

        switch(c)
        {
            case '0': autopoll_ = false; break;
            case '1': autopoll_ = true; break;
            default: return ERROR;
        }

        return OK;
    }

    template <class CharIter>
    CharIter alerts(CharIter out)
    {
        *out++ = 'F';
        // DEBT: Extract char_type from iter
        using num_put = estd::num_put<char, CharIter>;
        num_put np; // DEBT: It feels like one of these days he might end up requiring an instance.  Not today though
        estd::ios_base fmt;

        fmt.setf(estd::ios_base::hex | estd::ios_base::uppercase,
            estd::ios_base::basefield);
        fmt.width(2);

        out = np.put(out, fmt, '0', (uint8_t)alerts_);

        return out;
    }

    const char* alerts()
    {
        char* out = alerts(to_host_buffer);
        *out = 0;
        return to_host_buffer;
    }

public:
    const char* parse(estd::string_view s);
};

}}}}
