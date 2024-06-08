#include <QCanBus>
#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include <j1939/qt/ca/network.h>
#include <j1939/qt/transport.h>

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
    engine.loadFromModule("oel", "Main");

    embr::j1939::qt::ca::Network network;

    if (QCanBus::instance()->plugins().contains(QStringLiteral("virtualcan"))) {
        QCanBusDevice *device = QCanBus::instance()->createDevice(
            QStringLiteral("virtualcan"), QStringLiteral("can0"));

        QObject::connect(device, &QCanBusDevice::stateChanged, [&network, device]
            (QCanBusDevice::CanBusDeviceState state)
        {
            if(state == QCanBusDevice::ConnectedState)
                network.start(device);
        });

        device->connectDevice();
    }

    return app.exec();
}
