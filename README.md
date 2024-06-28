# J1939 C++ Library

Welcome to `embr::j1939` library!  What makes this particular library interesting
compared to some others?  In a word: specialization

We've gone all-in with C++ specialization to bring you a robust, intuitive yet lean implementation.  For example, setting up and sending a `cab message 1` PDU is as easy as:

```c++
pdu<pgns::cm1> p(sa, da, null_t{});

p.requested_percent_fan_speed(speed);

transport_traits::send(t, p);
```

DBC files are useful, but hardly translate well to highly constrained devices.  `embr::j1939` features:

* Network address negotiation
* Transport Protocol (~1.7k packets)
* Compile-time optimized units such as percentages, volts, kilometers, etc. by way of `embr` lib

## Quick Start: CMake

This is the preferred and easiest bringup approach.

`git submodule update --init --recursive`

## Quick Start: ESP-IDF

ESP-IDF is the primary target of this library.

## Quick Start: PlatformIO

TBD create instructions for making a new project from scratch

# Infrastructure

## Primary Targets

Supported:

| Platform | MCU            | Board 
| -------- | -------------- | -----
| Arduino  | AVR            | Promicro
| Arduino  | M4 SAMC (TBD)  | Adafruit Feather CAN M4
| ESP-IDF  | ESP32          | Many

## Secondary Targets

Although this library is tuned for embedded use, it compiles under most GCC and Clang environments.  Secondary targets include:

* Linux (see catch unit testing area)
* Qt

# 3. 