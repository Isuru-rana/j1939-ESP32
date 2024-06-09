#include <QCanBus>
#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include <j1939/qt/cs/generic.h>
#include <j1939/qt/cs/network.h>
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

    embr::j1939::qt::cs::Network network;
    embr::j1939::qt::cs::Generic generic;

    if (QCanBus::instance()->plugins().contains(QStringLiteral("virtualcan"))) {
        QCanBusDevice *device = QCanBus::instance()->createDevice(
            QStringLiteral("virtualcan"), QStringLiteral("can0"));

        // Just for the time being.  Looks like this is for OTHERS connected to can0... ?
        device->setConfigurationParameter(QCanBusDevice::LoopbackKey, true);
        device->setConfigurationParameter(QCanBusDevice::ReceiveOwnKey, true);

        // Getting nothing here just yet.  Loopback not working?
        QObject::connect(device, &QCanBusDevice::framesReceived, device, [&]
        {
            while(device->framesAvailable() > 0)
            {
                QCanBusFrame frame = device->readFrame();

                qDebug() << "Got frame:" << Qt::hex << frame.frameId();

                generic.frameReceived(frame);
                network.frameReceived(frame);
            }
        });

        QObject::connect(device, &QCanBusDevice::stateChanged, [&, device]
            (QCanBusDevice::CanBusDeviceState state)
        {
            if(state == QCanBusDevice::ConnectedState)
            {
                network.start(device);
            }

        });

        device->connectDevice();
    }

    return app.exec();
}
