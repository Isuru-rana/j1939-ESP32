#include "j1939/qt/cs/generic.h"

#include <j1939/data_field/all.hpp>

#include <j1939/internal/dispatcher/incoming.hpp>

namespace embr::j1939::qt::cs { inline namespace v1 {

void Generic::frameReceived(const QCanBusFrame& frame)
{
    embr::can::qt_transport t;

    j1939::process_incoming(*this, t, frame);
}

}}
