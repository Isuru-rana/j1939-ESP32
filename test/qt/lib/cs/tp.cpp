#include <estd/charconv.h>
#include <estd/string.h>

#include <j1939/internal/dispatcher/incoming2.hpp>
#include <j1939/state-machines/transport_protocol.hpp>

#include "j1939/qt/cs/tp.h"

namespace embr::j1939::qt::cs { inline namespace v1 {

TransportProtocol::TransportProtocol(QObject *parent) :
    Base(parent)
{
    // start with one idle
    reserve();
    connect(&timer_, &QTimer::timeout, this, &TransportProtocol::processOutgoing2);
}

bool TransportProtocol::Session::frameReceived(QCanBusDevice* device, const QCanBusFrame& f)
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

            // DEBT: Somehow it's necessary to force const here for
            // overloaded const accessor below to get picked up
            const auto& tp = tp_;

            const bool last_one = tp.responder().last_one();

            // DEBT:
            // Scheduled timeout is for listening to originator,
            // But we need to process now to evaluate things like:
            // - last packet eval
            // - send cts to orig for batched mode
            tp_.process_outgoing(t, ctx);
            return last_one;
        }

        default:    return false;
    }
}


void TransportProtocol::frameReceived(QCanBusDevice* device, const QCanBusFrame& f)
{
    unsigned idle_count = 0;
    Session* new_sess = nullptr;
    decltype(sessions_)::iterator it;
    time_point next_event = time_point::max();

    // DEBT: process_incoming needs an lvalue
    transport_type t{device};

    for(it = sessions_.begin(); it != sessions_.end(); )
    {
        std::unique_ptr<Session>& _sess = *it;
        Session& sess = *_sess.get();

        if(sess.tp_.state() == states::IDLE)
        {
            if(++idle_count > 1)
            {
                it = sessions_.erase(it);
                continue;
            }
        }

        ++it;

        bool last_one = sess.frameReceived(device, f);

        if(last_one)
        {
            qDebug() << "TransportProtocol::frameReceived" << sess.buffer_;
            // DEBT: Send proper can_id
            emit packetReceived(0, sess.buffer_);
            // DEBT: Wait for this to go idle again
            sess.buffer_.clear();
        }

        switch(sess.tp_.state())
        {
            case states::RESPONDER_RECEIVED_RTS:
            case states::RESPONDER_RECEIVED_BAM:
                new_sess = &sess;
                break;

            default:
                break;
        }

        // TODO: IIRC we can and do have our own std lhs estd rhs + and - operators.
        // They either aren't quite right, or not existing as I recall them.  They definitely weren't build out
        // much
        //next_event_ = std::min(next_event_, sess.tp_.next_event());
        /*
        time_point next_event = std::min(next_event_, sess.tp_.next_event());

        if(next_event != time_point::min())
            next_event_ = next_event;   */

        constexpr const time_point none;
        const time_point tp_next_event = sess.tp_.next_event();
        if(tp_next_event != none)
            next_event = std::min(next_event, tp_next_event);
    }

    // NOTE: Beware, all this gets activated even when it's not tp traffic!

    // If no idle sessions are around to pick up potential new incoming connection,
    // set one up.
    if(idle_count == 0)
    {
        reserve();  // gauruntees at least 1 idle is present
        //Session& sess = reserve();

        // FIX: At the moment, duplicates new sessions
        //sess.frameReceived(device, f);
    }

    next_event_ = next_event == time_point::max() ? time_point{} : next_event;  // DEBT

    schedule(next_event_);
}


auto TransportProtocol::reserve() -> Session&
{
    qDebug() << "TransportProtocol::reserve: current count:" << sessions_.size();

    return *sessions_.emplace_back(new Session).get();
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


void TransportProtocol::Session::processOutgoing(QCanBusDevice* device, const context_type& ctx)
{
    transport_type t{device};

    if(tp_.state() == states::IDLE) return;

    // DEBT: Upgrade to_string to handle different bases
    auto str = estd::to_string((int)tp_.state());

    qDebug()
        << "TransportProtocol::Session::processOutgoing phase 1:"
        << this
        << j1939::to_string(tp_.state(), str.data())
        << " next:" << std::chrono::duration_cast<milliseconds>(tp_.next_event() - Base::startup);

#if FEATURE_EMBR_J1939_TP_FUTURE
    unsigned guard = 0;

    while(tp_.elapsed(ctx) && ++guard < 5)
#else
    const time_point next_event = tp_.next_event();

    if(ctx.current >= next_event)
#endif
    {
        // DEBT: state machine itself doesn't filter process_outgoing by next_event, but maybe
        // it should.  Decision is because some consumers themselves are schedulers and only call
        // SM when it's time.  Smells of premature optimization
        tp_.process_outgoing(t, ctx);

        qDebug()
            << "TransportProtocol::Session::processOutgoing phase 2:"
            << this
            << j1939::to_string(tp_.state());
            //<< ctx.current.time_since_epoch();
    }

#if FEATURE_EMBR_J1939_TP_FUTURE
    if(guard == 5)  qDebug() << "GUARD HIT";
#endif
}


void TransportProtocol::processOutgoing(QCanBusDevice* device)
{
    //qDebug() << "TransportProtocol::processOutgoing";

    // DEBT: Slight debt, it really would be better to do 'now' as close as possible
    // to process_outgoing, but debugging is easier if we capture a 'now' point in time
    time_point now = clock::now();
    time_point next_event = time_point::max();

    for(std::unique_ptr<Session>& _sess : sessions_)
    {
        Session& sess = *_sess.get();
        // FIX: semi-race condition with sess and friends here vs frameReceived
        // because outgoing traffic immediately triggers frameReceived
        context_type ctx(now, sess.sa_);
        sess.processOutgoing(device, ctx);

        // DEBT: 'none' value may be better served as 'max()'
        constexpr time_point none;

        time_point tp_next_event = sess.tp_.next_event();

        if(tp_next_event != none)
            next_event = std::min(tp_next_event, next_event);

        /*
        if(next_event_ != none)
            next_event = std::min(next_event_, next_event);

        if(next_event != none)
            next_event_ = next_event;   */
    }

    next_event_ = next_event == time_point::max() ? time_point{} : next_event;  // DEBT

    schedule(next_event_);
}


}}
