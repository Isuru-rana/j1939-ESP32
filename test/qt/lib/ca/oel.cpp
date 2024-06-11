#include <j1939/internal/dispatcher/incoming.hpp>
#include <j1939/state-machines/lcmd.hpp>

#include "j1939/qt/ca/oel.h"


namespace embr::j1939::qt::ca { inline namespace v1 {

void OEL::frameReceived(QCanBusDevice* device, const QCanBusFrame& frame)
{
    transport_type t{device};
    //process_incoming(lcmd_, t, frame);
}

}}
