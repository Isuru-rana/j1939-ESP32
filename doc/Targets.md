# Targets Guide

This document indicates how to add new hardware targets

Document v0.1

Conforms to [Policy](Policy.md) v0.1

# 1. Scope and Design Description

Since this library is largely hands-off for hardware specifity,
targeting new MCU/peripherals has only a few steps.  Primarily involved
are specialiations pertaining to CAN bus:

1. `transport` class
2. `frame_traits` specialization

> NOTE: This document is a WIP and may be missing information

# 2. Implementation

## 2.1. transport class

This class is a rudimentary HAL around CAN capabilities.  The following
instance methods are *required*:

1. send
2. receive
3. good

## 2.2. frame_traits specialization

There are two kinds of `frame_traits`:

1. `embr::can::frame_traits`
2. `embr::j1939::frame_traits`

Only #1 is discussed here.  #2 behaves well as is with no
further attention

The following static methods are *required*:

1. `uint32_t id(frame)`
2. `void id(frame, uint32_t)`
3. `uint8_t* payload(frame)`
4. `void length(frame, uint8_t)`
5. `uint8_t length(frame)`
6. `void extended(frame, bool)`
