#include "j1939/qt/cs/generic.h"

//#include <j1939/data_field/all.hpp>   // DEBT: Want to do this, missing a bunch of spn::traits<>::name() fields though
#include <j1939/data_field/ccvs.hpp>
//#include <j1939/data_field/cm1.hpp>
#include <j1939/data_field/oel.hpp>
#include <j1939/data_field/lighting_command.hpp>
#include <j1939/data_field/network.hpp>
#include <j1939/data_field/vep1.hpp>

#include <j1939/internal/dispatcher/incoming.hpp>

#include "j1939/qt/cs/generic.hpp"

namespace embr::j1939::qt::cs { inline namespace v1 {

void Generic::frameReceived(QCanBusDevice*, const QCanBusFrame& frame)
{
    embr::can::qt_transport t;

    bool processed = j1939::process_incoming(*this, t, frame);
    if(!processed)
    {
        // unrecognized PGN
        j1939::can_id can_id(frame.frameId());

        auto p = new Pdu(can_id, this);

        // TODO: At the moment, datafield has no provision for unspecialized behavior

        emit pduReceived(p);
    }
}


// DEBT: I can't remember if there's a different, better place to emit a PDU - I think
// this is a good one though
void Generic::send(const Pdu*)
{
    // TBD
}


void Base::schedule(time_point next_event)
{
    constexpr time_point none;

    if(next_event == none) return;

    const time_point now = clock::now();

    if(next_event < now)  return;

    milliseconds interval(
        std::chrono::duration_cast<milliseconds>(
            next_event - now));

    qDebug()
        << "Base::schedule interval:"
        << interval;

    timer_.start(interval);
}

}}
