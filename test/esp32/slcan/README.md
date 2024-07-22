# Overview

Not a j1939 specific thing!

USB-CAN bridge

Document v0.1
See LAWICEL / SLCAN linux support

# 1. Design Goals

This firmware is a USB-CAN bridge by way of SLCAN protocol.  Technically supported by Linux, macOS and Windows (See Section 2)

## 1.1. SLCAN protocol implementation

Also known as LAWICEL, this protocol is a USB implementation of the older UART-CAN protocol.  Supported:

* RTR requests
* Auto-polling on or off (default on)
* Error/Alert interrogation

# 2. Infrastructure

* Linux (native)
* macOS (https://www.mac-can.com/)
* Windows (https://github.com/uv-software/SerialCAN)

# 3. Observations & Opinions

## 3.1. Unexpected host->USB traffic

Seeing a ^G appear during slcan_attach, so far don't have a clue what that is.

## 3.2. Linux can utilities

Linux SLCAN and friends are really quite extensive, including a bunch of J1939 test tools [2]

## 3.3. Dev on ESP32S3

So far I can't quite get USJ + console to place nice together.  I feel there is a way to do it, however

# Results

* 20JUN24 - ESP32C6 DevKit - PASS
* 21JUN24 - ESP32S3 LilyGo QT - PASS

# References

1. https://accesio.com/MANUALS/CAN232FD_Reference.html
2. https://github.com/linux-can/can-utils/tree/master