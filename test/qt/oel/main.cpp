#include <QCanBus>
#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include <j1939/qt/cs/generic.h>
#include <j1939/qt/cs/network.h>
#include <j1939/qt/transport.h>
#include <j1939/qt/session.h>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    qmlRegisterType<embr::j1939::qt::DataField>("j1939", 1, 0, "DataField");
    qmlRegisterType<embr::j1939::qt::Pdu>("j1939", 1, 0, "Pdu");
    //qmlRegisterType<embr::j1939::qt::Session>("j1939", 1, 0, "Session");
    qmlRegisterType<embr::j1939::qt::cs::v1::Generic>("j1939.cs", 1, 0, "Generic");

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection
    );

    embr::j1939::qt::Session session(&engine);

    qmlRegisterSingletonInstance("j1939", 1, 0, "Session", &session);

    engine.loadFromModule("oel", "Main");

    //embr::j1939::qt::cs::Network network;
    //embr::j1939::qt::cs::Generic generic;

    if (QCanBus::instance()->plugins().contains(QStringLiteral("virtualcan"))) {
        QCanBusDevice *device = QCanBus::instance()->createDevice(
            QStringLiteral("virtualcan"), QStringLiteral("can0"));

        // Just for the time being.  Looks like this is for OTHERS connected to can0... ?
        device->setConfigurationParameter(QCanBusDevice::LoopbackKey, true);
        device->setConfigurationParameter(QCanBusDevice::ReceiveOwnKey, true);

        session.setDevice(device);

        /*
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

        }); */

        device->connectDevice();
    }

    return app.exec();
}
