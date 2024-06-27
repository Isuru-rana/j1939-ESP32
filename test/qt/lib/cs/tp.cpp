#include "j1939/qt/cs/tp.h"

namespace embr::j1939::qt::cs { inline namespace v1 {

TransportProtocol::TransportProtocol(QObject *parent) :
    Base(parent)
{
}


void TransportProtocol::frameReceived(QCanBusDevice*, const QCanBusFrame&)
{

}


}}
