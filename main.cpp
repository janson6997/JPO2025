/**
 * @file main.cpp
 * @brief Main entry point for the air quality monitoring application.
 * @author Jan Podborowski
 * @date 2025-04-18
 *
 * This file contains the main function that initializes the Qt application
 * and sets up the QML engine.
 */

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "mainwindow.h"

/**
 * @brief Main function of the application.
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line arguments.
 * @return Application exit code.
 *
 * Initializes the Qt application, sets up the MainWindow instance,
 * and Natalities, and loads the main QML file.
 */
int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    // Utworzenie instancji MainWindow
    MainWindow mainWindow;

    QQmlApplicationEngine engine;
    // Udostępnienie MainWindow w QML jako "mainWindow"
    engine.rootContext()->setContextProperty("mainWindow", &mainWindow);

    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
                         if (!obj && url == objUrl)
                             QCoreApplication::exit(-1);
                     }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
