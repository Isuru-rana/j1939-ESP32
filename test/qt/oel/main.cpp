#include <QCanBus>
#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include <j1939/qt/plugin.h>

#include <j1939/qt/session.h>
#include <j1939/qt/ca/ccvs.h>
#include <j1939/qt/ca/oel.h>
#include <j1939/qt/ca/lighting_command.h>

using namespace embr;

#define SOCKETCAN_ENABLED 1

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

    j1939::qt::Plugin::init();

    auto session = new embr::j1939::qt::Session(&engine);

    auto oel = new j1939::qt::ca::OEL(session);
    auto lcmd = new j1939::qt::ca::LightingCommand(session);
    auto ccvs = new j1939::qt::ca::CCVS(session);

    session->clients().push_back(oel);
    session->clients().push_back(lcmd);
    session->clients().push_back(ccvs);

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
#endif

        // Just for the time being.  Looks like this is for OTHERS connected to can0... ?
        //device->setConfigurationParameter(QCanBusDevice::LoopbackKey, true);
        device->setConfigurationParameter(QCanBusDevice::ReceiveOwnKey, true);

        session->setDevice(device);
        oel->start(device);
        lcmd->start(device);
        ccvs->start(device);

        device->connectDevice();
    }

    return app.exec();
}
