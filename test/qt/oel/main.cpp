#include <QCanBus>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include <j1939/qt/plugin.h>

#include <j1939/qt/session.h>
#include <j1939/qt/ca/bjm.h>
#include <j1939/qt/ca/ccvs.h>
#include <j1939/qt/ca/oel.h>
#include <j1939/qt/ca/lighting_command.h>
#include <j1939/qt/cs/tp.h>

using namespace embr;

#define SOCKETCAN_ENABLED 0

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

    j1939::qt::Plugin::init(runtime);

    auto session = new j1939::qt::Session(runtime);

    auto bjm = new j1939::qt::ca::BJM(session);
    auto oel = new j1939::qt::ca::OEL(session);
    auto lcmd = new j1939::qt::ca::LightingCommand(session);
    auto ccvs = new j1939::qt::ca::CCVS(session);

    session->clients().push_back(oel);
    session->clients().push_back(lcmd);
    session->clients().push_back(ccvs);
    session->clients().push_back(bjm);

    runtime->caQmlFactory()->map(
        &j1939::qt::ca::v1::BJM::staticMetaObject, "oel/BJM.qml");
    runtime->caQmlFactory()->map(
        &j1939::qt::ca::v1::LightingCommand::staticMetaObject, "oel/LCMD.qml");
    runtime->caQmlFactory()->map(
        &j1939::qt::ca::v1::OEL::staticMetaObject, "oel/OEL.qml");
    runtime->caQmlFactory()->map(
        &j1939::qt::ca::v1::CCVS::staticMetaObject, "oel/CCVS.qml");

    qmlRegisterSingletonInstance("j1939", 1, 0, "Session", session);

    engine.rootContext()->setContextObject(new embr::j1939::qt::v1::API(&app));

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
        bjm->start(device);
        oel->start(device);
        lcmd->start(device);
        ccvs->start(device);

        session->tp()->start(device);

        device->connectDevice();
    }

    return app.exec();
}
