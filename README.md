# J1939 C++ Library

# 1. Introduction, Scope and Goals

Welcome to `embr::j1939` library!  What makes this particular library interesting
compared to some others?  In a word: specialization

We've gone all-in with C++ specialization to bring you a robust, intuitive yet lean implementation.  For example, setting up and sending a `cab message 1` PDU is as easy as:

```c++
pdu<pgns::cm1> p(sa, da, null_t{});

p.requested_percent_fan_speed(speed);

transport_traits::send(t, p);
```

DBC files are excellent, but hardly translate well to highly constrained devices.

## 1.1. Quick Start: CMake

This is the preferred and easiest bringup approach.

## 1.2. Quick Start: ESP-IDF

## 1.3. Quick Start: PlatformIO

TBD create instructions for making a new project from scratch

# 2. Infrastructure

## 2.1. Primary Targets

Supported:

| Platform | MCU            | Board 
| -------- | -------------- | -----
| Arduino  | AVR            | Promicro
| Arduino  | M4 SAMC (TBD)  | Adafruit Feather CAN M4
| ESP-IDF  | ESP32          | Many

## 2.2. Secondary Targets

Although this library is tuned for embedded use, it compiles under most GCC and Clang environments.  Secondary targets include:

* Linux (see catch unit testing area)
* Qt

# 3. 