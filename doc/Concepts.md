# Library Specific Notions and Ideas and Architecture

## Dispatcher

To translate a pgn into a compile time pdu<pgn>, we route through a big
ol' switch statement referred to as a dispatcher.  Similar to a factory
pattern.

## Controller Service

J1939 has a strict definition of "Controller Application":

* TBD get some bullet points as well as supporting references

Things like network address acquisition, etc are thought of as constituent parts of a controller application, so we refer to these as a "Controller Service" aka `cs`

Fundamentally, a `cs` comes down to three methods:

* `process_incoming_default`
* `process_incoming`
* `process_outgoing`
