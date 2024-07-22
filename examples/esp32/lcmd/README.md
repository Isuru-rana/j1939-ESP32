# Overview

Lighting Command CA example

Document v0.1

Conforms to [Policy](../../../doc/Policy.md) v0.1

# 1. Design Goals and Scope

This example is to show how to control GPIO-driven lights via the following J1939 
commands:

* OEL
* CCVS
* LCMD

The firmware has double duty:

* LCMD Sink (receiver) for activating GPIO
* LCMD Source (sender) for emitting timed flasher messages, etc

These are configurable via the menuconfig mechanism

# 2. Infrastructure