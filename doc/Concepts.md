# Library Specific Notions and Ideas and Architecture

> NOTE: We're not talking about c++20 concepts here

Document v0.1

## 1. Dispatcher

To translate a runtime pgn into a compile time pgn, we route through a big
ol' switch statement referred to as a dispatcher.  Similar to a factory
pattern, a functor is passed in requiring two signatures:

* PGN supported: `auto operator()(in_place_pgn&lt;pgns&gt;)`
* PGN unsupported: `auto operator()(pgns)`

PGN is supported if:

1. It is part of the built-in switch statement
2. It is not excluded by policy (See section 3.1.)

## 2. Incoming Processor

This builds on aforemention dispatcher and does two additional things:

1. Creates a specialized PDU associated with PGN
2. Invokes `process_incoming` or `process_incoming_default`

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
