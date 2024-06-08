#include <QCanBus>
#include <QGuiApplication>
#include <QQmlApplicationEngine>

//#include <j1939/slots/enum.h>
#include <j1939/qt/ca/network.h>

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


    if (QCanBus::instance()->plugins().contains(QStringLiteral("virtualcan"))) {
        QCanBusDevice *device = QCanBus::instance()->createDevice(
            QStringLiteral("virtualcan"), QStringLiteral("can0"));
        device->connectDevice();
    }

    return app.exec();
}
