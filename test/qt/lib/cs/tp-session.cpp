#include <memory>

#include <estd/charconv.h>
#include <estd/string.h>

#include <j1939/internal/dispatcher/incoming2.hpp>
#include <j1939/state-machines/transport_protocol.hpp>

#include "j1939/qt/cs/tp.h"

namespace embr::j1939::qt::cs { inline namespace v1 {

static void debugOut(const sm::tp::v0::originator_state& originator)
{
    qDebug()
        << "originator"
        << "bam:" << originator.bam()
        << "last_seq:" << originator.last_sequence()
        << "sent_all:" << originator.sent_everything();
}


static void debugOut(const sm::tp::v0::responder_state& responder)
{
    qDebug()
        << "responder"
        << "bam:" << responder.bam()
        << "last_seq:" << responder.seq()
        << "last_one:" << responder.last_one()
        << "max_pkt:" << responder.max_packets();
}

template <class TimePoint, class Policy>
static void debugOut(const sm::transport_protocol<TimePoint, Policy>& tp)
{
    switch(tp.role())
    {
        case sm::tp::base::ROLE_ORIGINATOR:
            debugOut(tp.originator());
            break;

        case sm::tp::base::ROLE_RESPONDER:
            debugOut(tp.responder());
            break;

        default: break;
    }
}


// DEBT: We're too greedy and design intends to answer the call of ANY RTS.  That will
// interrupt regular multi node tp flow.  Time is ticking before this is a FIX
bool TransportProtocol::Session::frameReceived(QCanBusDevice* device, const QCanBusFrame& f)
{
    // DEBT: process_incoming needs an lvalue
    transport_type t{device};
    // DEBT: Somehow it's necessary to force const here for
    // overloaded const accessor below to get picked up
    const auto& tp = tp_;

    context_type ctx(clock::now(), sa_);

    result r = j1939::v2::process_incoming(tp_, t, f, ctx);

    //debugOut(tp_);

    bool last_one = false;

    switch(tp_.state())
    {
        case states::RESPONDER_RECEIVING_DT:
        {
            // NOTE: tp_.payload() side-effect moves us to RESPONDER_RECEIVED_DT
            const estd::span<const uint8_t> p(tp_.payload());
            buffer_.append((const char*)p.data(), p.size());

#if FEATURE_EMBR_J1939_CS_ADV_RESULT == 0
            const bool last_one = tp.responder().last_one();

            // DEBT:
            // Scheduled timeout is for listening to originator,
            // But we need to process now to evaluate things like:
            // - last packet eval
            // - send cts to orig for batched mode
            tp_.process_outgoing(t, ctx);
            return last_one;
#else
            last_one = tp.responder().last_one();
            break;
#endif
        }

        default:
#if FEATURE_EMBR_J1939_CS_ADV_RESULT == 0
            return false;
#else
            break;
#endif
    }

#if FEATURE_EMBR_J1939_CS_ADV_RESULT == 1
    processOutgoing(device, ctx, r);
    return last_one;
#endif
}


void TransportProtocol::Session::processOutgoing(
    QCanBusDevice* device,
    const context_type& ctx,
    result r)
{
    transport_type t{device};

    {
        QMutexLocker ml(&mutex_);

        if(tp_.state() == states::IDLE ||
            tp_.state() == states::OFFLINE) return;

        if(processing_ == true) return;
    }

    // DEBT: Upgrade to_string to handle different bases
    auto str = estd::to_string((int)tp_.state());
    QString _next =
        tp_.next_event() != time_point{} ?
        (QString::number(std::chrono::duration_cast<milliseconds>(tp_.next_event() - Base::startup).count()) + "ms") :
        "null";

    qDebug().noquote()
        << "TransportProtocol::Session::processOutgoing phase 1:"
        << this
        << j1939::to_string(tp_.state(), str.data())
        << " next:" << _next;

#if FEATURE_EMBR_J1939_TP_FUTURE
    unsigned guard = 0;

    while((r.immediate || tp_.elapsed(ctx)) && ++guard < 5)
#else
    const time_point next_event = tp_.next_event();

    if(ctx.current >= next_event)
#endif
    {
        mutex_.lock();
        processing_ = true;
        mutex_.unlock();

        // DEBT: state machine itself doesn't filter process_outgoing by next_event, but maybe
        // it should.  Decision is because some consumers themselves are schedulers and only call
        // SM when it's time.  Smells of premature optimization
        r = tp_.process_outgoing(t, ctx);

        mutex_.lock();
        processing_ = false;
        mutex_.unlock();

        qDebug()
            << "TransportProtocol::Session::processOutgoing phase 2:"
            << this
            << j1939::to_string(tp_.state());
        //<< ctx.current.time_since_epoch();
        //debugOut(tp_);
    }

#if FEATURE_EMBR_J1939_TP_FUTURE
    if(guard == 5)  qDebug() << "GUARD HIT";
#endif
}



}}
