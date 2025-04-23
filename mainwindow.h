/**
 * @file mainwindow.h
 * @brief Header file for MainWindow and Station classes.
 * @author Jan Podborowski
 * @date 2025-04-18
 *
 * This file defines the MainWindow and Station classes used in the air quality monitoring application.
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QObject>
#include <QGeoCoordinate>
#include <QQmlListProperty>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>
#include <QJsonDocument>
#include <QDateTime>

/**
 * @class Station
 * @brief Represents an air quality monitoring station.
 *
 * This class encapsulates the properties of a station, such as its ID, name, location, and search status.
 */
class Station : public QObject {
    Q_OBJECT
    Q_PROPERTY(int stationId READ stationId CONSTANT)
    Q_PROPERTY(QString stationName READ stationName CONSTANT)
    Q_PROPERTY(QString cityName READ cityName CONSTANT)
    Q_PROPERTY(QString address READ address CONSTANT)
    Q_PROPERTY(double lat READ lat CONSTANT)
    Q_PROPERTY(double lon READ lon CONSTANT)
    Q_PROPERTY(bool isSearched READ isSearched NOTIFY isSearchedChanged)

public:
    /**
     * @brief Constructs a Station object.
     * @param id Station ID.
     * @param name Station name.
     * @param city City name.
     * @param addr Station address.
     * @param latitude Latitude coordinate.
     * @param longitude Longitude coordinate.
     * @param searched Search status.
     * @param parent Parent QObject.
     */
    Station(int id, QString name, QString city, QString addr, double latitude, double longitude, bool searched, QObject *parent = nullptr)
        : QObject(parent), m_stationId(id), m_stationName(name), m_cityName(city), m_address(addr), m_lat(latitude), m_lon(longitude), m_isSearched(searched) {}

    /**
     * @brief Gets the station ID.
     * @return Station ID.
     */
    int stationId() const { return m_stationId; }

    /**
     * @brief Gets the station name.
     * @return Station name.
     */
    QString stationName() const { return m_stationName; }

    /**
     * @brief Gets the city name.
     * @return City name.
     */
    QString cityName() const { return m_cityName; }

    /**
     * @brief Gets the station address.
     * @return Station address.
     */
    QString address() const { return m_address; }

    /**
     * @brief Gets the latitude.
     * @return Latitude coordinate.
     */
    double lat() const { return m_lat; }

    /**
     * @brief Gets the longitude.
     * @return Longitude coordinate.
     */
    double lon() const { return m_lon; }

    /**
     * @brief Gets the search status.
     * @return True if the station is marked as searched.
     */
    bool isSearched() const { return m_isSearched; }

    /**
     * @brief Sets the search status.
     * @param searched New search status.
     */
    void setIsSearched(bool searched) {
        if (m_isSearched != searched) {
            m_isSearched = searched;
            emit isSearchedChanged();
        }
    }

signals:
    /**
     * @brief Emitted when the search status changes.
     */
    void isSearchedChanged();

private:
    int m_stationId;            ///< Station ID
    QString m_stationName;      ///< Station name
    QString m_cityName;         ///< City name
    QString m_address;          ///< Station address
    double m_lat;               ///< Latitude
    double m_lon;               ///< Longitude
    bool m_isSearched;          ///< Search status
};

/**
 * @class MainWindow
 * @brief Main application logic for managing air quality stations.
 *
 * This class handles network requests, station data, and sensor data, and exposes them to QML.
 */
class MainWindow : public QObject {
    Q_OBJECT
    Q_PROPERTY(QGeoCoordinate mapCenter READ mapCenter NOTIFY mapCenterChanged)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(QQmlListProperty<Station> stations READ stations NOTIFY stationsChanged)
    Q_PROPERTY(QQmlListProperty<Station> allStations READ allStations NOTIFY allStationsChanged)
    Q_PROPERTY(QVariantList sensors READ sensors NOTIFY sensorsChanged)
    Q_PROPERTY(QVariantMap sensorData READ sensorData WRITE setSensorData NOTIFY sensorDataChanged)

public:
    /**
     * @brief Constructs a MainWindow object.
     * @param parent Parent QObject.
     */
    explicit MainWindow(QObject *parent = nullptr);

    /**
     * @brief Gets the current map center.
     * @return Map center coordinates.
     */
    QGeoCoordinate mapCenter() const { return m_mapCenter; }

    /**
     * @brief Gets the current status message.
     * @return Status message.
     */
    QString status() const { return m_status; }

    /**
     * @brief Gets the list of searched stations.
     * @return List of searched stations.
     */
    QQmlListProperty<Station> stations() { return QQmlListProperty<Station>(this, &m_stations); }

    /**
     * @brief Gets the list of all stations.
     * @return List of all stations.
     */
    QQmlListProperty<Station> allStations() { return QQmlListProperty<Station>(this, &m_allStations); }

    /**
     * @brief Gets the list of sensors.
     * @return List of sensors.
     */
    QVariantList sensors() const { return m_sensors; }

    /**
     * @brief Gets the sensor data.
     * @return Sensor data map.
     */
    QVariantMap sensorData() const { return m_sensorData; }

    /**
     * @brief Sets the sensor data.
     * @param data New sensor data.
     */
    void setSensorData(const QVariantMap &data) {
        if (m_sensorData != data) {
            m_sensorData = data;
            emit sensorDataChanged();
        }
    }

public slots:
    /**
     * @brief Searches for stations in a given city.
     * @param city City name to search for.
     */
    void searchCity(const QString &city);

    /**
     * @brief Fetches sensors for a station.
     * @param stationId Station ID.
     */
    void fetchSensors(int stationId);

    /**
     * @brief Fetches data for a sensor.
     * @param sensorId Sensor ID.
     */
    void fetchSensorData(int sensorId);

    /**
     * @brief Updates the search status of a station.
     * @param stationId Station ID.
     * @param isSearched New search status.
     */
    void updateStationSearchStatus(int stationId, bool isSearched);

    /**
     * @brief Removes data for a sensor.
     * @param sensorId Sensor ID.
     */
    void removeSensorData(int sensorId);

    /**
     * @brief Saves station data to a file.
     * @param stationId Station ID.
     * @param cityName City name.
     * @param address Station address.
     */
    void saveStationData(int stationId, const QString &cityName, const QString &address);

signals:
    /**
     * @brief Emitted when the map center changes.
     */
    void mapCenterChanged();

    /**
     * @brief Emitted when the status message changes.
     */
    void statusChanged();

    /**
     * @brief Emitted when the list of searched stations changes.
     */
    void stationsChanged();

    /**
     * @brief Emitted when the list of all stations changes.
     */
    void allStationsChanged();

    /**
     * @brief Emitted when the list of sensors changes.
     */
    void sensorsChanged();

    /**
     * @brief Emitted when sensor data changes.
     */
    void sensorDataChanged();

private slots:
    /**
     * @brief Handles geocode API reply.
     * @param reply Network reply.
     * @param searchedCity Searched city name.
     */
    void onGeocodeReply(QNetworkReply *reply, const QString &searchedCity);

    /**
     * @brief Handles stations API reply.
     * @param reply Network reply.
     */
    void onStationsReply(QNetworkReply *reply);

    /**
     * @brief Handles sensors API reply.
     * @param reply Network reply.
     */
    void onSensorsReply(QNetworkReply *reply);

    /**
     * @brief Handles sensor data API reply.
     * @param reply Network reply.
     * @param sensorId Sensor ID.
     */
    void onSensorDataReply(QNetworkReply *reply, int sensorId);

private:
    QGeoCoordinate m_mapCenter;         ///< Current map center
    QString m_status;                   ///< Current status message
    QList<Station*> m_stations;         ///< List of searched stations
    QList<Station*> m_allStations;      ///< List of all stations
    QVariantList m_sensors;             ///< List of sensors
    QVariantMap m_sensorData;           ///< Sensor data
    QNetworkAccessManager *m_networkManager; ///< Network manager for API requests
};

#endif // MAINWINDOW_H
