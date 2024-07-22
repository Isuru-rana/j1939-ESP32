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

#define SOCKETCAN_ENABLED 0
#define DIAGNOSTIC 0

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection
    );

    auto runtime = new j1939::qt::Runtime(&engine);

    app_init(runtime);

    auto session = new j1939::qt::Session(runtime);

    auto cm1 = new j1939::qt::ca::CM1(session);
    auto bjm = new j1939::qt::ca::BJM(session);
    auto oel = new j1939::qt::ca::OEL(session);
    auto lcmd = new j1939::qt::ca::LightingCommand(session);
    auto ccvs = new j1939::qt::ca::CCVS(session);

#if DIAGNOSTIC == 0
    session->clients().push_back(oel);
    session->clients().push_back(cm1);
#endif
    session->clients().push_back(lcmd);
    session->clients().push_back(ccvs);
#if DIAGNOSTIC == 0
    session->clients().push_back(bjm);
#endif

    qmlRegisterSingletonInstance("j1939", 1, 0, "Session", session);

    engine.loadFromModule("oel", "Main");

#if SOCKETCAN_ENABLED
    if (QCanBus::instance()->plugins().contains(QStringLiteral("socketcan")))
    {
        QCanBusDevice *device = QCanBus::instance()->createDevice(
            QStringLiteral("socketcan"), QStringLiteral("can0"));
#else
    if (QCanBus::instance()->plugins().contains(QStringLiteral("virtualcan")))
    {
        QCanBusDevice *device = QCanBus::instance()->createDevice(
            QStringLiteral("virtualcan"), QStringLiteral("can0"));

        // Just for the time being.  Looks like this is for OTHERS connected to can0... ?
        device->setConfigurationParameter(QCanBusDevice::LoopbackKey, true);
#endif

        // Just for the time being.  Looks like this is for OTHERS connected to can0... ?
        device->setConfigurationParameter(QCanBusDevice::ReceiveOwnKey, true);

        session->setDevice(device);
#if DIAGNOSTIC == 0
        bjm->start(device);
        oel->start(device);
        cm1->start(device);
#endif
        lcmd->start(device);
        ccvs->start(device);

        session->tp()->start(device);

        device->connectDevice();
    }

    return app.exec();
}
