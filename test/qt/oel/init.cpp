#include <QCanBus>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include <j1939/qt/plugin.h>

#include <j1939/qt/session.h>
#include <j1939/qt/ca/bjm.h>
#include <j1939/qt/ca/cm1.h>
#include <j1939/qt/ca/ccvs.h>
#include <j1939/qt/ca/oel.h>
#include <j1939/qt/ca/lighting_command.h>
#include <j1939/qt/cs/tp.h>

#include "init.h"

using namespace embr;

void app_init(j1939::qt::v1::Runtime* runtime)
{
    j1939::qt::Plugin::init(runtime);

    runtime->caQmlFactory()->map(
        &j1939::qt::ca::v1::BJM::staticMetaObject, "oel/BJM.qml");
    runtime->caQmlFactory()->map(
        &j1939::qt::ca::v1::CM1::staticMetaObject, "oel/CM1.qml");
    runtime->caQmlFactory()->map(
        &j1939::qt::ca::v1::LightingCommand::staticMetaObject, "oel/LCMD.qml");
    runtime->caQmlFactory()->map(
        &j1939::qt::ca::v1::OEL::staticMetaObject, "oel/OEL.qml");
    runtime->caQmlFactory()->map(
        &j1939::qt::ca::v1::CCVS::staticMetaObject, "oel/CCVS.qml");

    runtime->engine()->rootContext()->setContextObject(new embr::j1939::qt::v1::API(runtime));
}
