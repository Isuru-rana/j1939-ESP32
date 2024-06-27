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
    //using context = sm::v0::transport_protocol::context;

    //context ctx();

    for(Session& sess : sessions_)
    {
        //internal::v2::process_incoming(sess.tp_, transport_type{device}, f);
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
