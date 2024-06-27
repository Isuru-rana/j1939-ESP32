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
    using sm_type = sm::v0::transport_protocol;
    // DEBT: Heavy debt, need context to fully support time_point
    using context_type = sm_type::context; //<clock::time_point>;

    // Tracked according to:
    // - source address when responder
    // - dest address when originator
    struct Session
    {
        sm::v0::transport_protocol tp_;
        // Theoretically some kind of stream/pipe would be interesting here.
        // Practically, ~1.7k is the maximum size, so lots of in memory buffers are appropriate
        QByteArray buffer_;

        // If originating, we track sa here (since we're a pool)
        uint8_t sa_;
    };

    std::vector<Session> sessions_;

    Session& reserve();

    Q_OBJECT

public:
    TransportProtocol(QObject* parent = nullptr);

    void frameReceived(QCanBusDevice*, const QCanBusFrame&) override;

    void broadcast(uint8_t sa, pgns pgn,const QByteArray&);
    void respond(uint8_t sa, uint8_t da, pgns pgn, const QByteArray&);

    // TODO: This is only for the rare case of request whose payload is > 8 bytes
    void request(uint8_t sa, uint8_t da, pgns) {}

signals:
    void packetReceived(can_id, QByteArray);
};

}}
