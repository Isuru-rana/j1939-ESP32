#pragma once

#include <estd/string_view.h>
#include <can/reference.h>

namespace embr { namespace can { namespace slcan { inline namespace v0 {

namespace impl {

struct base
{
    static constexpr unsigned bitrates_[] =
        { 10, 20, 50, 100, 125, 250, 500, 800, 1000 };
};

struct loopback : base
{
    const char*  bitrate(unsigned v)
    {
        return "\r";
    }
};

}

template <class Impl = impl::loopback>
class parser : impl::base   // DEBT
{
    Impl impl_;

    static constexpr const char* OK = "\r";
    static constexpr const char* ERROR = "\7";
    static constexpr const char* OK_NEW = "z\r";

    const char* transmit(estd::string_view, bool extended, bool rtr) { return ERROR; }

#if UNIT_TESTING
public:
#else
protected:
#endif

    Impl& impl() { return impl_; }

    template <class Transport>
    void package(Transport& t)
    {

    }

    const char* bitrate(estd::string_view s)
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
                break;

            case 'L':       // open CAN channel (listen only)
                break;

            case 'C':       // close CAN channel
                break;

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
