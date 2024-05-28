#pragma once

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
#define FEATURE_EMBR_J1939_TP_CONTEXT_NEXT 0
#endif

#ifndef FEATURE_EMBR_J1939_TP_AUTO_PAYLOAD
#define FEATURE_EMBR_J1939_TP_AUTO_PAYLOAD 1
#endif
