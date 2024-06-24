#include <QtQml>

// Specifically we include this so that to_string has as much available to it as possible
#include <j1939/data_field/all.hpp>

#include "j1939/qt/plugin.h"

#include <j1939/qt/cs/generic.h>
#include <j1939/qt/cs/network.h>
#include <j1939/qt/transport.h>
#include <j1939/qt/session.h>

#include <j1939/qt/ca/ccvs.h>
#include <j1939/qt/ca/oel.h>
#include <j1939/qt/ca/lighting_command.h>

#include "j1939/qt/pdu.h"

namespace embr::j1939::qt { inline namespace v1 {

void Plugin::init()
{
    qmlRegisterType<embr::j1939::qt::DataField>("j1939", 1, 0, "DataField");
    qmlRegisterType<embr::j1939::qt::Pdu>("j1939", 1, 0, "Pdu");
    //qmlRegisterType<embr::j1939::qt::Session>("j1939", 1, 0, "Session");
    qmlRegisterType<embr::j1939::qt::cs::v1::Generic>("j1939.cs", 1, 0, "Generic");
    qmlRegisterType<embr::j1939::qt::cs::v1::Network>("j1939.cs", 1, 0, "Network");
    qmlRegisterType<embr::j1939::qt::ca::v1::LightingCommand>("j1939.ca", 1, 0, "LCMD");
    qmlRegisterType<embr::j1939::qt::ca::v1::OEL>("j1939.ca", 1, 0, "OEL");
    qmlRegisterType<embr::j1939::qt::ca::v1::CCVS>("j1939.ca", 1, 0, "CCVS");
}

QString API::to_string(pgns p, bool abbrev)
{
    if(abbrev)
        return internal::dispatch<internal::dispatch_default_policy>(pgn_to_string_functor<true>{}, p);
    else
        return j1939::to_string(p);
}

}}
