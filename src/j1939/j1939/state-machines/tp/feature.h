#pragma once

// Force a runtime error (assert/abort) if invalid state is reached.
// Be advised, these should be exceptional, as opposed to validation-style errors
// In particular, that means external inputs such as outside CAN traffic MUST NOT create a
// state in which this feature then asserts/aborts.  This is only to catch our
// own programmatic failures, not input validation issues.
#define FEATURE_EMBR_J1939_STRICT_STATES 1

#define FEATURE_EMBR_J1939_STRICT_PROTOCOL 1

// NOTE: Discretely enabling/disabling originator & responder is an optimization for
// thingns like AVR with very little space.  It seemed this would squeeze a little more
// than actually splitting into two separate state machines.  Not entirely sure though,
// and certainly two state machines would be cleaner.

#ifndef FEATURE_EMBR_J1939_TP_ORIGINATOR
#define FEATURE_EMBR_J1939_TP_ORIGINATOR 1
#endif

#ifndef FEATURE_EMBR_J1939_TP_RESPONDER
#define FEATURE_EMBR_J1939_TP_RESPONDER 1
#endif

// Include in context indication of next process_outgoing event
// Feature not ready yet
#ifndef FEATURE_EMBR_J1939_TP_CONTEXT_NEXT
#define FEATURE_EMBR_J1939_TP_CONTEXT_NEXT 1
#endif

// Runtime selectable auto-advance through payload in originator role
#ifndef FEATURE_EMBR_J1939_TP_AUTO_PAYLOAD
#define FEATURE_EMBR_J1939_TP_AUTO_PAYLOAD 1
#endif

// Intermediate feature:
// 1: Track next event by absolute time, calculating beforehand (preferred)
// 0: Track next event by last event time, calculating on request
// Feature not ready yet
#ifndef FEATURE_EMBR_J1939_TP_FUTURE
#define FEATURE_EMBR_J1939_TP_FUTURE 0
#endif
