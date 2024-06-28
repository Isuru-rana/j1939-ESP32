# J1939 C++ Library

Welcome to `embr::j1939` library!  

Features include:

* J939-21 Network support:
    * Network address negotiation
    * Transport Protocol (~1.7k packets)
* Compile-time optimized units such as percentages, volts, kilometers, etc. by way of `embr` lib

What makes this particular library interesting compared to some others?  In a word: specialization.

We've gone all-in with C++ specialization to bring you a robust, intuitive yet lean implementation.  For example, setting up and sending a `cab message 1` PDU is as easy as:

```c++
pdu<pgns::cm1> p(sa, da, null_t{});

p.requested_percent_fan_speed(speed);

transport_traits::send(t, p);
```

DBC files are useful, but hardly translate well to highly constrained devices.  With `embr::j1939`,
compile-time traits are available for you to fold expression over, interrogate on a case by case
basis, or ignore completely - "only pay for what you use"

## Quick Start

* `git submodule update --init --recursive`
* Naturally you'll need a CAN transciever.  See [TBD add local desc link which itself has local
desc and also links out to other guides]

### Quick Start: ESP-IDF

ESP-IDF is the primary target of this library.

See `examples/esp32/lcmd_sink` (TBD, example not yet existing - make a simple GPIO blinker responder)

### Quick Start: CMake

This is the easiest bringup approach.

### Quick Start: PlatformIO

I have yet to crack the nut to make platformio work smoothly with local libraries.  Therefore,
usage in this context is complicated.  See `test/arduino/lcmd_sink`

TBD create instructions for making a new project from scratch

## Infrastructure

### Primary Targets

Supported:

| Platform | MCU            | Board 
| -------- | -------------- | -----
| Arduino  | AVR            | Promicro
| Arduino  | M4 SAMC (TBD)  | Adafruit Feather CAN M4
| ESP-IDF  | ESP32          | Many

### Secondary Targets

Although this library is tuned for embedded use, it compiles under most GCC and Clang environments.  Secondary targets include:

* Linux (see catch unit testing area)
* Qt

## Extras

### SLCAN firmware (USB-CAN bridge)

This implements the SLCAN (LAWICEL) protocol for ESP-IDF.
Linux `slcand` happily speaks to this firmware.  From there any SocketCAN tool is
theoretically usable.  Works with all CAN, not just J1939

* Tested OK with Wireshark and Qt `QCanDeviceBus` (TBD fix name)
* Tested OK with ESP32C6 and ESP32S3

Find this under `test/esp32/slcan`

### Interesting Hardware

* Rejsacan
* ENGH-45 (TBD check name)
* Adafruit Feather M4 CAN

TBD

### External Links & Special Thanks

[Rejsacan]
[Jetbrains]
