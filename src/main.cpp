#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include "bridge/AppController.h"

int main(int argc, char *argv[]) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QGuiApplication app(argc, argv);
    app.setOrganizationName(QStringLiteral("ShanXueXi"));
    app.setApplicationName(QStringLiteral("闪学习"));
    app.setApplicationVersion(QStringLiteral("1.0"));

    QQuickStyle::setStyle(QStringLiteral("Basic"));

    shanxuexi::AppController controller;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("appController"), &controller);

    const QUrl url(QStringLiteral("qrc:/qml/Main.qml"));
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);

    engine.load(url);
    return app.exec();
}
