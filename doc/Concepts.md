# Library Specific Notions and Ideas and Architecture

> NOTE: We're not talking about c++20 concepts here

Document v0.1

## 1. Dispatcher

To translate a runtime pgn into a compile time pgn, we route through a big
ol' switch statement referred to as a dispatcher.  Similar to a factory
pattern.

Functor is passed in requiring two signatures:

* in_place_t<pgns>
* pgns 

### 1.1. Frame Resolution

Aforementioned switch statement creates a compile-time specialized
PDU associated with incoming PGN.

### 1.2. Functor behavior

## 2. Incoming Processor

## 3. Controller Service

J1939 has a strict definition of "Controller Application":

* TBD get some bullet points as well as supporting references

Things like network address acquisition, etc are thought of as constituent parts of a controller application, so we refer to these as a "Controller Service" aka `cs`

Fundamentally, a `cs` comes down to three methods:

* `process_incoming_default` - fallback for frames not resolved to PDU
* `process_incoming`
* `process_outgoing`

### 3.1. CS: policy

Dispatcher's big switch statement can cause some serious bloat.  This is mitigated
via policy whitelisting or blacklisting, one can filter which pgns flow into a CS.
