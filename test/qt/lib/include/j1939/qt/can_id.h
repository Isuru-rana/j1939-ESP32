#pragma once

#include <QObject>

#include <j1939/can_id.h>

namespace embr::j1939::qt { inline namespace v1 {

class CanId
{
    can_id id_{0};

    Q_GADGET

    Q_PROPERTY(pgns pgn READ pgn)

public:
    pgns pgn() const
    {

    }
};

}}
