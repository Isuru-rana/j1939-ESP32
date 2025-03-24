#pragma once

#include "../../data_field/transport_protocol.hpp"

namespace embr { namespace j1939 { namespace sm { namespace tp { inline namespace v0 {

enum errors
{
    ORIGINATOR_ERROR_MISMATCHED_PGM
};

struct enum_base
{
    using modes = pdu<pgns::tp_cm>::modes;
    using abort_reasons = pdu<pgns::tp_cm>::abort_reasons;

    enum roles
    {
        ROLE_UNINITIALIZED = 0,
        ROLE_ORIGINATOR = 1,
        ROLE_RESPONDER = 2
    };


    // EXPERIMENTAL, not used
    enum frame_errors
    {
        FRAME_NOMINAL,  // A-OK
        // Generic
        FRAME_ERROR,
        FRAME_TIMEOUT,
        FRAME_WARN,
        FRAME_INVALID_STATE
    };

    // EXPERIMENTAL, not used
    enum frame_states
    {
        FRAME_IDLE,
        FRAME_RECEIVING,
        FRAME_RECEIVED,
        FRAME_SENDING,
        FRAME_SENT
    };

    // EXPERIMENTAL, not used
    enum frame_types
    {
        FRAME_CTS,
        FRAME_RTS,
        FRAME_ACK,
        FRAME_ABORT,

        FRAME_DT,
    };

    // EXPERIMENTAL, not used - consider eventually merging with embr service
    // architecture
    struct frame_tracker
    {
        roles role_ : 4;
        frame_states state_ : 4;
        frame_types type_ : 4;
        frame_errors error_ : 4;
    };
};


}}}}}
