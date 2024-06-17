#include <estd/string_view.h>

#include <driver/twai.h>

#include <stdio.h>


class Parser
{
    static constexpr const char* OK = "\r";
    static constexpr const char* ERROR = "\7";
    static constexpr const char* OK_NEW = "z\r";

    const char* transmit(estd::string_view, bool extended, bool rtr) { return ERROR; }

public:
    const char* parse(estd::string_view s)
    {
        char c = s[0];
        const bool sz1 = s.size() == 1;

        estd::string_view param = s.substr(1);

        switch(c)
        {
            case 'S':       // setup bitrate
                break;

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
    }
};

void parser()
{

}

extern "C" void app_main(void)
{

}
