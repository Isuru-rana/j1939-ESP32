#pragma once

#include <QMutex>
#include <QTimer>

#include <j1939/cas/internal/prng_address_manager.h>
#include <j1939/state-machines/transport_protocol.h>

#include "../can_id.h"
#include "../transport.h"
#include "base.h"

// DEBT: Not really a CA.  As far as this Qt wrapper goes, not really a
// state machine other.  I suppose it sort of represents an actual transport
// at this level.

namespace embr::j1939::qt::cs { inline namespace v1 {

// pool of transport protocols
class TransportProtocol : public Base
{
    using sm_type = sm::v0::transport_protocol<time_point>;
    // DEBT: Heavy debt, need context to fully support time_point
    using context_type = sm_type::context; //<clock::time_point>;
    using states = sm_type::states;
    using result = j1939::cs::v1::base::result;

    // Tracked according to:
    // - source address when responder
    // - dest address when originator
    struct Session
    {
        sm_type tp_;
        // Theoretically some kind of stream/pipe would be interesting here.
        // Practically, ~1.7k is the maximum size, so lots of in memory buffers are appropriate
        QByteArray buffer_;

        // If originating, we track sa here (since we're a pool & state machine doesn't track this)
        uint8_t sa_ = addresses::null;

        QMutex mutex_;

        // NOTE: Consider storing QCanBusDevice* here for multiple transport outs

        // DEBT: returns whether entire buffer is received.  Would prefer to interrogate
        // responder().last_one() once we work out forced process_outgoing DEBT seen in frameReceived
        bool frameReceived(QCanBusDevice *, const QCanBusFrame &);
        void processOutgoing(QCanBusDevice *, const context_type&);
        void send(addr_type sa, addr_type da, pgns pgn, const QByteArray& v);
    };

    using session_type = std::shared_ptr<Session>;

    // DEBT: Consider this might be just a formality now since 'schedule' does the heavy lifting
    time_point next_event_;

    // Protects 'sessions_'
    QMutex mutex_;
    // DEBT: Using pointers instead of values because surreptitiously ::send cascades out to
    // frameReceived which in turn MT-changes vector and sometimes modifies Session values
    std::vector<session_type> sessions_;
    session_type offline_candidate_;
    session_type idle_;

    using iterator = std::vector<session_type>::iterator;

    QCanBusDevice* device_ = nullptr;

    Session& reserve();

    void processOutgoing(QCanBusDevice*);
    // DEBT: Fixup naming, just naming this so 'connect' deosn't get confused
    void processOutgoing2()
    {
        processOutgoing(device_);
    }
    void send(uint8_t sa, uint8_t da, pgns pgn, const QByteArray&);

    Q_OBJECT


public:
    TransportProtocol(QObject* parent = nullptr);

    void frameReceived(QCanBusDevice*, const QCanBusFrame&) override;

    void broadcast(uint8_t sa, pgns pgn,const QByteArray& v)
    {
        send(sa, addresses::global, pgn, v);
    }

    void respond(uint8_t sa, uint8_t da, pgns pgn, const QByteArray& v)
    {
        send(sa, da, pgn, v);
    }


    Q_INVOKABLE void send(addr_type sa, addr_type da, pgns pgn, const QString& v)
    {
        send(sa, da, pgn, v.toUtf8());
    }

    Q_INVOKABLE void broadcast(uint8_t sa, pgns pgn, const QString& v)
    {
        broadcast(sa, pgn, v.toUtf8());
    }

    // Listen for incoming tp:cm's on a particular address (think socket bind)
    Q_INVOKABLE void listen(addr_type address);


    // TODO: This is only for the rare case of request whose payload is > 8 bytes
    void request(uint8_t sa, uint8_t da, pgns) {}

    void start(QCanBusDevice* device)
    {
        device_ = device;
    }

signals:
    void packetReceived(CanId, QByteArray);
};

}}
