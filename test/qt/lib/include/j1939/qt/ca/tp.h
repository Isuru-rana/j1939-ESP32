#pragma once

#include <QTimer>

#include <j1939/cas/internal/prng_address_manager.h>
#include <j1939/state-machines/transport_protocol.h>

#include "../transport.h"

// DEBT: Not really a CA.  As far as this Qt wrapper goes, not really a
// state machine other.  I suppose it sort of represents an actual transport
// at this level.

namespace embr::j1939::qt::ca { inline namespace v1 {

class TransportProtocol : public QObject
{
    std::vector<sm::v0::transport_protocol> sessions_;

    Q_OBJECT

public:
    TransportProtocol(QObject* parent = nullptr);

signals:

};

}}
