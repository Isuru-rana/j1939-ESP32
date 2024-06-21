#pragma once

#include <estd/charconv.h>
#include <estd/iterator.h>
#include <estd/locale.h>
#include <estd/string_view.h>
#include <estd/iosfwd.h>

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
    static constexpr const char* OK = "\r";
    static constexpr const char* ERROR = "\7";
    static constexpr const char* OK_NEW = "z\r";

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
        BITRATE_1000K,
        BITRATE_UNSET
    };

    static constexpr unsigned bitrates_[] =
        { 10, 20, 50, 100, 125, 250, 500, 800, 1000 };

    // Would get fancy with chrono, but it's not really worth it
    static constexpr uint16_t timestamp_ms() { return 0xFFFF; }
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
    bool timestamps_ = false;

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
            if(!(r.ec == 0)) return r.ec;

            current += 2;

            *payload++ = v2;
        }

        return estd::errc{0};
    }

    template <class Streambuf, class Base>
    using ostream = estd::detail::basic_ostream<Streambuf, Base>;

    // NOTE: Serialize adds in prefix r/R/t/T
    template <class S, class B>
    ostream<S, B>& serialize(const frame_type& in, ostream<S, B>& out);

    // for 'parse' to use as its response buffer
    //char to_host_buffer[max_frame_str_size];

    template <class S, class B>
    ostream<S, B>& send_frame_to_host(ostream<S, B>& out)
    {
        if(!frames_to_send.empty())
        {
            serialize(frames_to_send.front(), out);

            frames_to_send.pop();
        }

        out << OK;

        return out;
    }


    // send out over CAN bus
    const char* transmit(view v, bool extended, bool rtr)
    {
        if(!impl().opened())    return ERROR;

        // Not supported yet
        if(rtr) return  ERROR;

        frame_type frame;

        frame_traits::rtr(frame, rtr);
        frame_traits::extended(frame, extended);

        deserialize(v, &frame, extended);

        return impl().transport().send(frame) ? OK : ERROR;
    }

    const char* bitrate(view s)
    {
        // "This command is only active if the CAN channel is closed."
        if(s.size() != 1 || impl().opened()) return ERROR;

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

    template <class S, class B>
    ostream<S, B>& alerts(ostream<S, B>& out)
    {
        out << 'F';

        out.setf(estd::ios_base::hex | estd::ios_base::uppercase,
            estd::ios_base::basefield);
        out.width(2);
        out.fill('0');

        out << (uint8_t)alerts_;

        return out << OK;
    }

public:
    const Impl& cimpl() const { return impl_; }
    bool autopoll() const { return autopoll_; }

    ///
    /// @tparam S
    /// @tparam B
    /// @param in
    /// @param out
    /// @return
    /// @remark char 13 (CR) MUST NOT be present for 'in'
    template <class S, class B>
    ostream<S, B>& parse(estd::string_view in, ostream<S, B>& out);

    // received from CAN bus
    // DEBT: Probably split this out into a kind of 'process_outgoing' statemachine-esque
    // emptying of frames_to_send during autopoll, so that we don't always pass in out.  Mainly
    // in service of potential flow control down the line
    template <class S, class B>
    auto on_receive(const frame_type& frame, ostream<S, B>& out) -> ostream<S, B>&
    {
        if(autopoll_)
        {
            serialize(frame, out);
        }
        else
        {
            frames_to_send.push(frame);
        }
        return out;
    }
};

}}}}
