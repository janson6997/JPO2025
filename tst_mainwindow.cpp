/**
 * @file tst_mainwindow.cpp
 * @brief Unit tests for MainWindow and Station classes.
 * @author Jan Kowalski
 * @date 2025-04-22
 *
 * This file contains unit tests for the MainWindow and Station classes using the Qt Test framework.
 */

#include <QtTest>
#include "mainwindow.h"

/**
 * @class TestMainWindow
 * @brief Test class for MainWindow and Station functionality.
 *
 * This class contains unit tests to verify the correctness of the MainWindow and Station classes.
 */
class TestMainWindow : public QObject
{
    Q_OBJECT

private slots:
    void testStationProperties()
    {
        Station station(1, "Test Station", "Test City", "Test Address", 50.0, 20.0, false, nullptr);
        QCOMPARE(station.stationId(), 1);
        QCOMPARE(station.stationName(), QString("Test Station"));
        QCOMPARE(station.cityName(), QString("Test City"));
        QCOMPARE(station.address(), QString("Test Address"));
        QCOMPARE(station.lat(), 50.0);
        QCOMPARE(station.lon(), 20.0);
        QCOMPARE(station.isSearched(), false);
    }

    void testMapCenter()
    {
        MainWindow mainWindow;
        QGeoCoordinate initial = mainWindow.mapCenter();
        QCOMPARE(initial.latitude(), 52.2297);
        QCOMPARE(initial.longitude(), 21.0122);

        mainWindow.searchCity("Krakow");
        QVERIFY(mainWindow.mapCenter().latitude() != 52.2297 || mainWindow.mapCenter().longitude() != 21.0122);
    }

    void testStationSearchStatus()
    {
        MainWindow mainWindow;
        Station *station = new Station(2, "Test Station", "Test City", "Test Address", 50.0, 20.0, false, &mainWindow);
        mainWindow.updateStationSearchStatus(2, true);
        QCOMPARE(station->isSearched(), true);
        mainWindow.updateStationSearchStatus(2, false);
        QCOMPARE(station->isSearched(), false);
    }

    void testSensorData()
    {
        MainWindow mainWindow;
        QVariantMap sensorData;
        sensorData["test"] = QVariantList{ QVariantMap{{"date", "2025-04-22"}, {"value", 10.0}} };
        mainWindow.setSensorData(sensorData);
        QCOMPARE(mainWindow.sensorData().size(), 1);
        QVERIFY(mainWindow.sensorData().contains("test"));

        mainWindow.removeSensorData(999);
        QCOMPARE(mainWindow.sensorData().size(), 1);

        mainWindow.setSensorData(QVariantMap());
        QCOMPARE(mainWindow.sensorData().size(), 0);
    }
};

QTEST_MAIN(TestMainWindow)
#include "tst_mainwindow.moc"
