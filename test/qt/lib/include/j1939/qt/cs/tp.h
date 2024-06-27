#pragma once

#include <QTimer>

#include <j1939/cas/internal/prng_address_manager.h>
#include <j1939/state-machines/transport_protocol.h>

#include "../transport.h"
#include "base.h"

// DEBT: Not really a CA.  As far as this Qt wrapper goes, not really a
// state machine other.  I suppose it sort of represents an actual transport
// at this level.

namespace embr::j1939::qt::cs { inline namespace v1 {

// pool of transport protocols
class TransportProtocol : public Base
{
    using clock = std::chrono::system_clock;
    using time_point = clock::time_point;
    using sm_type = sm::v0::transport_protocol<time_point>;
    // DEBT: Heavy debt, need context to fully support time_point
    using context_type = sm_type::context; //<clock::time_point>;
    using states = sm_type::states;

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
        uint8_t sa_;
    };

    // DEBT: Use a priority queue here
    time_point next_event_;

    std::vector<Session> sessions_;

    Session& reserve();

    Q_OBJECT

    void send(uint8_t sa, uint8_t da, pgns pgn, const QByteArray&);

public:
    TransportProtocol(QObject* parent = nullptr);

    void frameReceived(QCanBusDevice*, const QCanBusFrame&) override;
    void processOutgoing(QCanBusDevice*);

    void broadcast(uint8_t sa, pgns pgn,const QByteArray& v)
    {
        send(sa, addresses::global, pgn, v);
    }

    void respond(uint8_t sa, uint8_t da, pgns pgn, const QByteArray& v)
    {
        send(sa, da, pgn, v);
    }


    // TODO: This is only for the rare case of request whose payload is > 8 bytes
    void request(uint8_t sa, uint8_t da, pgns) {}

signals:
    void packetReceived(can_id, QByteArray);
};

}}
