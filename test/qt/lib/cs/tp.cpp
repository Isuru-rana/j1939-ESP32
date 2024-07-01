#include <estd/charconv.h>
#include <estd/string.h>

#include <j1939/internal/dispatcher/incoming2.hpp>
#include <j1939/state-machines/transport_protocol.hpp>

#include "j1939/qt/cs/tp.h"

namespace embr::j1939::qt::cs { inline namespace v1 {

TransportProtocol::TransportProtocol(QObject *parent) :
    Base(parent)
{
    connect(&timer_, &QTimer::timeout, this, &TransportProtocol::processOutgoing2);
}

void TransportProtocol::Session::frameReceived(QCanBusDevice* device, const QCanBusFrame& f)
{
    // DEBT: process_incoming needs an lvalue
    transport_type t{device};

    context_type ctx(clock::now(), sa_);

    internal::v2::process_incoming(tp_, t, f, ctx);

    switch(tp_.state())
    {
        case states::RESPONDER_RECEIVING_DT:
        {
            const estd::span<const uint8_t> p(tp_.payload());
            buffer_.append((const char*)p.data(), p.size());
            break;
        }

        default:    break;
    }
}


void TransportProtocol::frameReceived(QCanBusDevice* device, const QCanBusFrame& f)
{
    unsigned idle_count = 0;
    Session* first_idle = nullptr;
    Session* new_sess = nullptr;

    // DEBT: process_incoming needs an lvalue
    transport_type t{device};

    for(Session& sess : sessions_)
    {
        sess.frameReceived(device, f);

        switch(sess.tp_.state())
        {
            case states::RESPONDER_RECEIVED_BAM:
                new_sess = &sess;
                break;

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

            case states::IDLE:
                if(first_idle == nullptr)   first_idle = &sess;
                ++idle_count;
                break;

            default: break;
        }

        // TODO: IIRC we can and do have our own std lhs estd rhs + and - operators.
        // They either aren't quite right, or not existing as I recall them.  They definitely weren't build out
        // much
        //next_event_ = std::min(next_event_, sess.tp_.next_event());
        time_point next_event = std::min(next_event_, sess.tp_.next_event());

        if(next_event != time_point::min())
            next_event_ = next_event;
    }

    // If no idle sessions are around to pick up potential new incoming connection,
    // set one up.
    if(idle_count == 0)
    {
        Session& sess = reserve();

        // FIX: At the moment, duplicates new sessions
        sess.frameReceived(device, f);
    }
    else if(idle_count > 1)
    {
        //sessions_.erase(first_idle);
    }

    schedule(next_event_);
}


auto TransportProtocol::reserve() -> Session&
{
    qDebug() << "TransportProtocol::reserve: current count:" << sessions_.size();

    return sessions_.emplace_back();
}


void TransportProtocol::Session::send(uint8_t sa, uint8_t da, pgns pgn, const QByteArray& v)
{
    buffer_ = v;
    // FIX: auto payload not working for BAM
    tp_.initiate_originator(
        da, uint32_t(pgn),
        buffer_.data(),
        buffer_.size());
    sa_ = sa;
}



void TransportProtocol::send(uint8_t sa, uint8_t da, pgns pgn, const QByteArray& v)
{
    Session& sess = reserve();

    sess.send(sa, da, pgn, v);

    // DEBT: Brute force the kickoff
    transport_type t{device_};
    context_type ctx(clock::now(), sess.sa_);
    sess.tp_.process_outgoing(t, ctx);
    schedule(sess.tp_.next_event());
}


void TransportProtocol::Session::processOutgoing(QCanBusDevice* device)
{
    transport_type t{device};
    context_type ctx(clock::now(), sa_);
    const time_point next_event = tp_.next_event();

    if(tp_.state() == states::IDLE) return;

    auto str = estd::to_string((int)tp_.state());

    qDebug() << "TransportProtocol::Session::processOutgoing:" << this << j1939::to_string(tp_.state(), str.data());

    if(ctx.current >= next_event)
    {
        // DEBT: state machine itself doesn't filter process_outgoing by next_event, but maybe
        // it should.  Decision is because some consumers themselves are schedulers and only call
        // SM when it's time.  Smells of premature optimization
        tp_.process_outgoing(t, ctx);
    }
}


void TransportProtocol::processOutgoing(QCanBusDevice* device)
{
    //qDebug() << "TransportProtocol::processOutgoing";

    for(Session& sess : sessions_)
    {
        sess.processOutgoing(device);

        time_point next_event = std::min(next_event_, sess.tp_.next_event());

        if(next_event != time_point::min())
            next_event_ = next_event;
    }

    schedule(next_event_);
}


}}
