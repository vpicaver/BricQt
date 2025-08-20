#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "BricClient.h"
#include "BricMeasurementModel.h"

#ifdef Q_OS_IOS
extern "C" void requestBluetoothPermission();
#endif

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

#ifdef Q_OS_IOS
    requestBluetoothPermission();
#endif

    BricMeasurementModel* model = new BricMeasurementModel(&app);

    auto client = new BricClient(&app);
    QObject::connect(client, &BricClient::info, &app, [](const QString& s) {
        qInfo() << s;
    });
    QObject::connect(client, &BricClient::measurementReady, &app, [model](const BricMeasurement& m) {
        qInfo().noquote() << QStringLiteral("ref=%1 dist=%2m azi=%3° inc=%4° Temp=%5C type=%6")
        .arg(m.metadata.referenceIndex)
            .arg(m.primary.distanceMeters)
            .arg(m.primary.azimuthDegrees)
            .arg(m.primary.inclinationDegrees)
            .arg(m.metadata.temperatureCelsius)
            .arg(m.metadata.measurementType);
        model->add(m);
    });
    client->startScan();


    QQmlApplicationEngine engine;
    auto context = engine.rootContext();

    QObject::connect(client, &BricClient::connected, context, [context]() {
        context->setContextProperty(QStringLiteral("connected"), true);
    });

    QObject::connect(client, &BricClient::disconnected, context, [context]() {
        context->setContextProperty(QStringLiteral("connected"), false);
    });

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.rootContext()->setContextProperty(QStringLiteral("bricModel"), QVariant::fromValue(model));
    engine.rootContext()->setContextProperty(QStringLiteral("connected"), false);
    engine.rootContext()->setContextProperty(QStringLiteral("bricClient"), QVariant::fromValue(client));
    engine.loadFromModule("BricQt", "Main");


    return app.exec();
}
