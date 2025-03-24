#pragma once

#include <tinyusb.h>
#include <tusb_cdc_acm.h>
#include <tusb.h>

#include <estd/streambuf.h>

// Initial guidance from
// https://github.com/espressif/esp-idf/blob/v5.1.4/examples/peripherals/usb/device/tusb_serial_device/main/tusb_serial_device_main.c
// https://github.com/hathach/tinyusb/blob/0.15.0/examples/device/cdc_dual_ports/src/usb_descriptors.c

// "Documentation"
// https://github.com/hathach/tinyusb/blob/0.15.0/src/class/cdc/cdc_device.h
namespace embr { namespace tinyusb { inline namespace v1 {

namespace impl {

// tud_cdc_n_write_available
// tud_cdc_n_write_flush

template <class CharTraits>
struct ocdc_streambuf
{
    uint8_t itf_;

    using traits_type = CharTraits;

    typedef typename traits_type::char_type char_type;
    typedef typename traits_type::int_type int_type;

protected:
    int_type overflow(int_type ch = traits_type::eof())
    {
        tud_cdc_n_write_char(itf_, ch);
        return {};
    }

public:
    // DEBT: Feels like a boilerplate that owning streambuf can
    // implement
    int_type sputc(char_type ch)
    {
        const int_type _ch = traits_type::to_int_type(ch);
        int_type result = overflow(_ch);
        if(result == traits_type::eof()) return traits_type::eof();
        return _ch;
    }

    estd::streamsize xsputn(const char_type* s, estd::streamsize count)
    {
        tud_cdc_n_write(itf_, s, count);
        return {};
    }
};

}

}}}
