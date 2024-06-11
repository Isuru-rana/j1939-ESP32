#include <QtQml>

#include "j1939/qt/plugin.h"

#include <j1939/qt/cs/generic.h>
#include <j1939/qt/cs/network.h>
#include <j1939/qt/transport.h>
#include <j1939/qt/session.h>
#include <j1939/qt/ca/oel.h>
#include <j1939/qt/ca/lighting_command.h>

namespace embr::j1939::qt {

void Plugin::init()
{
    qmlRegisterType<embr::j1939::qt::DataField>("j1939", 1, 0, "DataField");
    qmlRegisterType<embr::j1939::qt::Pdu>("j1939", 1, 0, "Pdu");
    //qmlRegisterType<embr::j1939::qt::Session>("j1939", 1, 0, "Session");
    qmlRegisterType<embr::j1939::qt::cs::v1::Generic>("j1939.cs", 1, 0, "Generic");
    qmlRegisterType<embr::j1939::qt::cs::v1::Network>("j1939.cs", 1, 0, "Network");
    qmlRegisterType<embr::j1939::qt::ca::v1::LightingCommand>("j1939.ca", 1, 0, "LCMD");
    qmlRegisterType<embr::j1939::qt::ca::v1::OEL>("j1939.ca", 1, 0, "OEL");
}

}
