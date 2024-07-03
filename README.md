# embr::J1939 C++ Library

![j1939 logo](doc/img/j1939.jpg "J1939")

Welcome!

If you're new to CAN bus or J1939, [check this out](https://www.csselectronics.com/pages/j1939-explained-simple-intro-tutorial) for some background

## Why?

What makes this particular library interesting?  In a word: specialization.

We've gone all-in with C++ specialization to bring you a robust, intuitive & lean implementation.  For example, setting up and sending a `cab message 1` PDU is as easy as:

```c++
pdu<pgns::cm1> p(sa, da, null_t{});

p.requested_percent_fan_speed(speed);

transport_traits::send(t, p);
```

DBC files are useful, but don't directly play well in constrained devices.  With `embr::j1939`,
compile-time traits are available for you to interrogate, or ignore completely - "only pay for what you use"

### Features include:

* J939-21 Network support:
    * Network address negotiation
    * Transport Protocol (1785b data field size)
* Compile-time optimized units such as percentages, volts, kilometers, etc. by way of `embr` lib
* Compile-time metadata for SPNs, PGNs including name, type, ownership, more
* Lean and highly portable.  No dynamic allocation.
* c++11 compliant

## Quick Start

* Initialize `estd` and `embr` via `git submodule update --init --recursive`
* Naturally you'll need a [Can Transceiver](doc/Transceiver.md)

### Quick Start: ESP-IDF

See `examples/esp32/lcmd_sink` (TBD, example not yet existing - make a simple GPIO blinker responder)

Use `idf.py menuconfig` to specify CAN speed and TX/RX pins via `embr` config menu item

### Quick Start: CMake

This is the easiest bringup approach.

### Quick Start: PlatformIO

I have yet to crack the nut to make platformio work smoothly with local libraries.  Therefore,
usage in this context is complicated.  See `test/arduino/lcmd_sink`

TBD create instructions for making a new project from scratch

### Other Quick Snippets

[Find more code snippets here](doc/Snippets.md)

## Infrastructure

### Primary Targets

Supported:

| Platform | MCU            | Board 
| -------- | -------------- | -----
| Arduino  | AVR            | Promicro
| Arduino  | M4 SAMC (TBD)  | Adafruit Feather CAN M4
| ESP-IDF  | ESP32          | Many

### Secondary Targets

Although this library is tuned for embedded use, it compiles under GCC and Clang environments.  Secondary targets include:

* Linux (see catch unit testing area)
* Qt/QML

## Extras

### SLCAN firmware (USB-CAN bridge)

This implements the SLCAN (LAWICEL) protocol for ESP-IDF.
Linux `slcand` happily speaks to this firmware.  From there any SocketCAN tool is
theoretically usable.  Works with all CAN, not just J1939

* Full read/write capability
* Auto-poll (default) as well as legacy polled mode
* Tested OK with Wireshark and Qt `QCanDeviceBus` (TBD fix name)
* Tested OK with ESP32C6 and ESP32S3

Find this under `test/esp32/slcan`

### Interesting Hardware

* Rejsacan
* ENGH-45 (TBD check name)
* Adafruit Feather M4 CAN

TBD

### External Links & Special Thanks

[RejsaCAN](https://github.com/MagnusThome/RejsaCAN-ESP32/tree/main)
[Jetbrains]
[Seeed Xiao]

---
*Document v0.1*
