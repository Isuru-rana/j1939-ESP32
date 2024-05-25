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
};


}}}}}
