/**
 * References:
 *
 * 1. CAN232_VER3_Manual
 * 2. https://accesio.com/MANUALS/CAN232FD_Reference.html
 */
#pragma once

#include <estd/charconv.h>
#include <estd/iterator.h>
#include <estd/locale.h>
#include <estd/string_view.h>
#include <estd/iosfwd.h>

#include <can/reference.h>
// DEBT: Move this loopback stuff elsewhere
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

enum slcan_policies
{
    SLCAN_AUTOPOLL_DYNAMIC,         // On or off (See X0 and X1)
    SLCAN_AUTOPOLL_ON,              // On always (See X1)

    SLCAN_POLICY_DEFAULT = SLCAN_AUTOPOLL_DYNAMIC
};

namespace impl {

// Transport abstraction is very hard.  Do up impl pattern for some auxiliary
// transport specifics rather than a full on transport abstraction

struct shared
{
    static constexpr const char* OK = "\r";
    static constexpr const char* ERROR = "\7";
    static constexpr const char* OK_AUTOPOLL = "z\r";

    enum open_modes
    {
        CLOSED,
        OPEN_NORMAL,
        OPEN_LISTEN,
        OPEN_ECHO,      // [2]
    };

    enum autostart_modes : uint8_t
    {
        AUTOSTART_NONE,
        AUTOSTART_NORMAL,
        AUTOSTART_LISTEN
    };

    enum alerts_type : uint8_t
    {
        ALERT_NONE,
        // Data overrun in CAN receive to host transfer. [2]
        ALERT_RX_FIFO_FULL      = 0x01,
        // Data overrun in receive by host send to CAN transfer. [2]
        ALERT_TX_FIFO_FULL      = 0x02,
        // (Extended) Unexpected characters between us and host [2]
        ALERT_DATA_STREAM       = 0x10,
        ALERT_BUS_PASSIVE       = 0x20,
        ALERT_ARBITRATION_LOST  = 0x40,
        ALERT_BUS_ERROR         = 0x80
    };


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


    // DEBT: Not right, this is a bitfield
    static const char* to_string(alerts_type v)
    {
        switch(v)
        {
            case ALERT_NONE:            return "None";
            case ALERT_RX_FIFO_FULL:    return "RX Overrun";
            case ALERT_TX_FIFO_FULL:    return "TX Overrun";
            case ALERT_BUS_ERROR:       return "Bus Error";
            case ALERT_BUS_PASSIVE:     return "Bus Passive";

            default:    return "N/A";
        }
    }
};



class base : public shared
{
public:


    // Would get fancy with chrono, but it's not really worth it
    static constexpr uint16_t timestamp_ms() { return 0xFFFF; }

    static constexpr alerts_type alerts() { return {}; }

    static constexpr slcan_policies policy = SLCAN_POLICY_DEFAULT;

    void init() {}

    static const char* autostart(autostart_modes)
    {
        return ERROR;
    }

    static autostart_modes autostart() { return AUTOSTART_NONE; }

protected:
    open_modes open_mode_ {};

    bitrates_enum bitrate_ { BITRATE_UNSET };

public:

    const char* bitrate(unsigned idx, unsigned rate)
    {
        bitrate_ = bitrates_enum(idx);

        return OK;
    }

    ATTR_NODISCARD constexpr unsigned bitrate() const { return bitrate_; }

    ATTR_NODISCARD bool opened() const
    {
        return open_mode_ == OPEN_NORMAL ||
            open_mode_ == OPEN_LISTEN;
    }

    ATTR_NODISCARD constexpr open_modes open_mode() const
    {
        return open_mode_;
    }
};

struct loopback : base
{
    using transport_type = can::loopback_transport;

    transport_type transport_;

    transport_type& transport() { return transport_; }

    static const char* open(bool listen_only)
    {
        return OK;
    }

    static const char* close()
    {
        return OK;
    }
};


// DEBT: Really this is a base helper, perhaps 'impl' namespace is incorrect
template <class Impl, class Enabled = void>
class parser {};

// DEBT: Optimize ->host frame queue to be here
template <class Impl>
class parser<Impl, estd::enable_if_t<Impl::policy & SLCAN_AUTOPOLL_DYNAMIC> >
{

};

}

template <ESTD_CPP_CONCEPT(concepts::Impl) Impl = impl::loopback>
class parser :
    impl::parser<Impl>,
    impl::shared
{
    Impl impl_;

public:
    using transport_type = typename Impl::transport_type;
    using frame_type = typename transport_type::frame;
    using frame_traits = can::frame_traits<frame_type>;

    // Approximately
    static constexpr const unsigned max_frame_str_size = 30;

    using view = estd::string_view;

#if UNIT_TESTING
public:
#else
protected:
#endif

    // TODO: Put in autoopen mode as per [2]

    uint8_t alerts_ {};
    // Original spec indicates this is false [1] but Linux slcan suite seems to presume true
    bool autopoll_ = false;
    bool timestamps_ = false;

    Impl& impl() { return impl_; }
    const Impl& impl() const { return impl_; }

    uint8_t alerts() const  // NOLINT
    {
        // TODO: "Bits clear on read" [2]
        return impl().alerts() | alerts_;
    }

    // Frames to send to host
    // Primarily useful for polled mode, but also if to host USB doesn't keep up for some
    // reason, can be helpful too.
    estd::layer1::queue<frame_type, 5> frames_to_send;

    // Turn ASCII representation into native frame
    template <class CharIt>
    estd::errc deserialize(CharIt in, frame_type* out, bool extended);

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

        // Not supported yet, but almost
        if(rtr) return  ERROR;

        frame_type frame;

        frame_traits::rtr(frame, rtr);
        frame_traits::extended(frame, extended);

        estd::errc r = deserialize(v.begin(), &frame, extended);

        if(r == 0)
            return impl().transport().send(frame) ?
                (autopoll() ? OK_AUTOPOLL : OK) : ERROR;
        else
        {
            alerts_ |= ALERT_DATA_STREAM;
            return ERROR;
        }
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

    // (Extended) status [2]
    // NOTE: Unclear what is meant by "8-bit", maybe character width?
    template <class S, class B>
    ostream<S, B>& status(ostream<S, B>& out)
    {
        out << 'f';
        switch(impl().open_mode())
        {
            case OPEN_LISTEN:       out << 'L'; break;
            case OPEN_NORMAL:       out << 'O'; break;
            case OPEN_ECHO:         out << 'E'; break;  // [2]
            case CLOSED:            out << 'C'; break;
        }

        // TODO: Incomplete

        if(impl().bitrate() == BITRATE_UNSET)
            out << '-';     // NOTE: Standard doesn't specify this, but it is implied
        else
            out.put(impl().bitrate() + '0');

        out << '-'; // regular CAN
        out.put(timestamps_ ? 'Z' : '-');
        out << (impl().autostart() == AUTOSTART_NONE ? '0' : '1');

        return out << OK;
    }

    template <class S, class B>
    ostream<S, B>& alerts(ostream<S, B>& out)
    {
        out << 'F';

        out.setf(estd::ios_base::hex | estd::ios_base::uppercase,
            estd::ios_base::basefield);
        out.width(2);
        out.fill('0');

        out << alerts();

        return out << OK;
    }

public:
    const Impl& cimpl() const { return impl_; }
    bool autopoll() const { return autopoll_; }
    void autopoll(bool v) { autopoll_ = v; }

    ///
    /// @tparam S Streambuf
    /// @tparam B ios base
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
            if(out.bad())
                alerts_ |= ALERT_RX_FIFO_FULL;
        }
        else
        {
            frames_to_send.push(frame);
        }
        return out;
    }

    // DEBT: Clumsy, RAII WRU?
    void init() { impl().init(); }
};

}}}}

inline const char* to_string(embr::can::slcan::v0::impl::shared::alerts_type a)
{
    return embr::can::slcan::v0::impl::shared::to_string(a);
}