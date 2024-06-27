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

class TransportProtocol : public Base
{
    // Tracked according to:
    // - source address when responder
    // - dest address when originator
    struct Session
    {
        sm::v0::transport_protocol tp_;
        // Theoretically some kind of stream/pipe would be interesting here.
        // Practically, ~1.7k is the maximum size, so lots of in memory buffers are appropriate
        QByteArray buffer_;
    };

    std::vector<Session> sessions_;

    Q_OBJECT

public:
    TransportProtocol(QObject* parent = nullptr);

    void frameReceived(QCanBusDevice*, const QCanBusFrame&) override;

signals:

};

}}
