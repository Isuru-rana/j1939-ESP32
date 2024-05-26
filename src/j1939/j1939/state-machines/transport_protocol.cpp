#include <estd/chrono.h>

#include "transport_protocol.hpp"

namespace embr { namespace j1939 {

const char* to_string(sm::v0::transport_protocol::states v)
{
    using states = sm::v0::transport_protocol::states;

    switch(v)
    {
        case states::IDLE:                      return "Idle";
        case states::ORIGINATOR_SENDING_BAM:    return "Sending BAM";
        case states::ORIGINATOR_SENT_BAM:       return "Sent BAM";
        case states::ORIGINATOR_SENDING_DT:     return "Sending DT";
        case states::ORIGINATOR_SENT_DT:        return "Sent DT";

        case states::RESPONDER_RECEIVING_DT:    return "Receiving DT";
        case states::RESPONDER_RECEIVED_DT:     return "Received DT";

        default:    return "N/A";
    }
}

namespace sm { namespace tp { inline namespace v0 {

void responder_state::prep_cts(pdu<pgns::tp_cm>& cm, uint8_t self_address) const
{
    cm.destination_address(originator_.source_address());
    cm.source_address(self_address);
    cm.control(modes::cts);
    cm.to_send(seq() + 1);
    //uint32_t pgn = responder().pgn();
    uint32_t pgn = originator_.payload().pgn();
    cm.payload().pgn(pgn);
}

}}}

}}
