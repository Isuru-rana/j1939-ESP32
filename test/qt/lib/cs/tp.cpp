#include <j1939/internal/dispatcher/incoming2.hpp>
#include <j1939/state-machines/transport_protocol.hpp>

#include "j1939/qt/cs/tp.h"

namespace embr::j1939::qt::cs { inline namespace v1 {

TransportProtocol::TransportProtocol(QObject *parent) :
    Base(parent)
{
}


void TransportProtocol::frameReceived(QCanBusDevice* device, const QCanBusFrame& f)
{
    // DEBT: process_incoming needs an lvalue
    transport_type t{device};

    for(Session& sess : sessions_)
    {
        context_type ctx(clock::now(), sess.sa_);

        internal::v2::process_incoming(sess.tp_, t, f, ctx);

        switch(sess.tp_.state())
        {
            case states::RESPONDER_RECEIVING_DT:
            {
                const estd::span<const uint8_t> p(sess.tp_.payload());
                sess.buffer_.append((const char*)p.data(), p.size());
                break;
            }

            case states::RESPONDER_SENT_EOM_ACK:
            {
                // DEBT: Send proper can_id
                emit packetReceived(0, sess.buffer_);
                // DEBT: Remove session
                break;
            }

            case states::ORIGINATOR_RECEIVED_EOM_ACK:
                // DEBT: Remove session
                break;

            default: break;
        }
    }
}


auto TransportProtocol::reserve() -> Session&
{
    return sessions_.emplace_back();
}


void TransportProtocol::broadcast(uint8_t sa, pgns pgn, const QByteArray& v)
{
    Session& sess = reserve();

    sess.buffer_ = v;
    sess.tp_.initiate_originator(j1939::addresses::global, uint32_t(pgn), v.data(), v.size());
}

void TransportProtocol::respond(uint8_t sa, uint8_t da, pgns pgn, const QByteArray& v)
{
    Session& sess = reserve();

    sess.buffer_ = v;
    sess.tp_.initiate_originator(da, uint32_t(pgn), v.data(), v.size());
}


}}
