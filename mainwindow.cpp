/**
 * @file mainwindow.cpp
 * @brief Implementation of the MainWindow class.
 * @author Jan Podborowski
 * @date 2025-04-20
 *
 * This file contains the implementation of the MainWindow class, which handles
 * the core logic for fetching and managing air quality data.
 */

#include "mainwindow.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QUrlQuery>
#include <QDebug>
#include <QFile>
#include <QDateTime>

/**
 * @brief Constructs a MainWindow object.
 * @param parent Parent QObject.
 *
 * Initializes the map center to Warsaw, sets the default status message,
 * and fetches all stations from the API.
 */
MainWindow::MainWindow(QObject *parent)
    : QObject(parent),
    m_mapCenter(52.2297, 21.0122), // Domyślnie Warszawa
    m_status("Wprowadź nazwę miasta i kliknij Szukaj"),
    m_networkManager(new QNetworkAccessManager(this))
{
    // Pobierz wszystkie stacje przy starcie
    QNetworkRequest request(QUrl("https://api.gios.gov.pl/pjp-api/rest/station/findAll"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkReply *reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onStationsReply(reply);
    });
}

/**
 * @brief Searches for stations in a given city.
 * @param city City name to search for.
 *
 * Sends a request to the Nominatim API to geocode the city and updates the map center.
 */
void MainWindow::searchCity(const QString &city)
{
    m_status = "Wyszukiwanie: " + city + "...";
    emit statusChanged();

    QUrlQuery query;
    query.addQueryItem("q", city);
    query.addQueryItem("format", "json");
    query.addQueryItem("limit", "1");

    QUrl url("https://nominatim.openstreetmap.org/search");
    url.setQuery(query);

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "ControlStationsApp/1.0");
    QNetworkReply *reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, city]() {
        onGeocodeReply(reply, city);
    });
}

/**
 * @brief Fetches sensors for a station.
 * @param stationId Station ID.
 *
 * Sends a request to the GIOŚ API to retrieve sensors for the specified station.
 */
void MainWindow::fetchSensors(int stationId)
{
    QUrl url(QString("https://api.gios.gov.pl/pjp-api/rest/station/sensors/%1").arg(stationId));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkReply *reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onSensorsReply(reply);
    });
}

/**
 * @brief Fetches data for a sensor.
 * @param sensorId Sensor ID.
 *
 * Sends a request to the GIOŚ API to retrieve data for the specified sensor.
 */
void MainWindow::fetchSensorData(int sensorId)
{
    QUrl url(QString("https://api.gios.gov.pl/pjp-api/rest/data/getData/%1").arg(sensorId));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkReply *reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, sensorId]() {
        onSensorDataReply(reply, sensorId);
    });
}

/**
 * @brief Updates the search status of a station.
 * @param stationId Station ID.
 * @param isSearched New search status.
 *
 * Updates the isSearched property for the specified station.
 */
void MainWindow::updateStationSearchStatus(int stationId, bool isSearched)
{
    for (Station *station : m_allStations) {
        if (station->stationId() == stationId) {
            station->setIsSearched(isSearched);
        }
    }
    emit allStationsChanged();
}

/**
 * @brief Removes data for a sensor.
 * @param sensorId Sensor ID.
 *
 * Removes the data associated with the specified sensor ID.
 */
void MainWindow::removeSensorData(int sensorId)
{
    m_sensorData.remove(QString::number(sensorId));
    emit sensorDataChanged();
}

/**
 * @brief Saves station data to a file.
 * @param stationId Station ID.
 * @param cityName City name.
 * @param address Station address.
 *
 * Saves the station and sensor data to a JSON file.
 */
void MainWindow::saveStationData(int stationId, const QString &cityName, const QString &address)
{
    // Find the station in allStations
    Station *station = nullptr;
    for (Station *s : m_allStations) {
        if (s->stationId() == stationId) {
            station = s;
            break;
        }
    }

    if (!station) {
        m_status = "Błąd: Stacja o ID " + QString::number(stationId) + " nie znaleziona.";
        emit statusChanged();
        return;
    }

    // Create JSON object
    QJsonObject jsonObj;
    jsonObj["stationId"] = stationId;
    jsonObj["stationName"] = station->stationName();
    jsonObj["cityName"] = cityName;
    jsonObj["address"] = address;
    jsonObj["latitude"] = station->lat();
    jsonObj["longitude"] = station->lon();
    jsonObj["saveDate"] = QDateTime::currentDateTime().toString(Qt::ISODate);

    // Add sensor data
    QJsonArray sensorsArray;
    for (const QVariant &sensorVariant : m_sensors) {
        QVariantMap sensorInfo = sensorVariant.toMap();
        int sensorId = sensorInfo["sensorId"].toInt();
        QString paramName = sensorInfo["paramName"].toString();

        QJsonObject sensorObj;
        sensorObj["sensorId"] = sensorId;
        sensorObj["paramName"] = paramName;

        // Add measurements
        QVariantList data = m_sensorData[QString::number(sensorId)].toList();
        QJsonArray measurementsArray;
        for (const QVariant &dataPoint : data) {
            QVariantMap dataMap = dataPoint.toMap();
            QJsonObject measurementObj;
            measurementObj["date"] = dataMap["date"].toString();
            measurementObj["value"] = dataMap["value"].toDouble();
            measurementsArray.append(measurementObj);
        }
        sensorObj["measurements"] = measurementsArray;
        sensorsArray.append(sensorObj);
    }
    jsonObj["sensors"] = sensorsArray;

    // Create JSON document
    QJsonDocument jsonDoc(jsonObj);

    // Generate filename based on station ID and timestamp
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString filename = QString("station_%1_%2.json").arg(stationId).arg(timestamp);

    // Save to file
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        m_status = "Błąd: Nie można otworzyć pliku do zapisu: " + filename;
        emit statusChanged();
        return;
    }

    file.write(jsonDoc.toJson(QJsonDocument::Indented));
    file.close();

    m_status = "Dane zapisano do pliku: " + filename;
    emit statusChanged();
}

/**
 * @brief Handles geocode API reply.
 * @param reply Network reply.
 * @param searchedCity Searched city name.
 *
 * Processes the response from the Nominatim API, updates the map center, and finds stations.
 */
void MainWindow::onGeocodeReply(QNetworkReply *reply, const QString &searchedCity)
{
    if (reply->error() != QNetworkReply::NoError) {
        m_status = "Błąd wyszukiwania: " + reply->errorString();
        emit statusChanged();
        reply->deleteLater();
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    QJsonArray results = doc.array();

    if (results.isEmpty()) {
        m_status = "Nie znaleziono miasta.";
        emit statusChanged();
        reply->deleteLater();
        return;
    }

    QJsonObject result = results.first().toObject();
    double lat = result["lat"].toString().toDouble();
    double lon = result["lon"].toString().toDouble();

    m_mapCenter = QGeoCoordinate(lat, lon);
    emit mapCenterChanged();

    // Wyczyść listę wyszukanych stacji
    m_stations.clear();
    // Resetuj flagę isSearched dla wszystkich stacji
    for (Station *station : m_allStations) {
        station->setIsSearched(false);
    }

    // Znajdź stacje w dokładnie wyszukanym mieście
    QString normalizedSearchedCity = searchedCity.toLower().simplified();
    for (Station *station : m_allStations) {
        QString stationCity = station->cityName().toLower().simplified();
        if (stationCity == normalizedSearchedCity) {
            Station *searchedStation = new Station(
                station->stationId(),
                station->stationName(),
                station->cityName(),
                station->address(),
                station->lat(),
                station->lon(),
                true,
                this
                );
            m_stations.append(searchedStation);
            updateStationSearchStatus(station->stationId(), true);
        }
    }

    if (m_stations.isEmpty()) {
        // Znajdź najbliższą stację
        Station *closestStation = nullptr;
        double minDistance = std::numeric_limits<double>::max();
        QGeoCoordinate cityCoord(lat, lon);

        for (Station *station : m_allStations) {
            QGeoCoordinate stationCoord(station->lat(), station->lon());
            double distance = cityCoord.distanceTo(stationCoord);
            if (distance < minDistance) {
                minDistance = distance;
                closestStation = station;
            }
        }

        if (closestStation) {
            Station *searchedStation = new Station(
                closestStation->stationId(),
                closestStation->stationName(),
                closestStation->cityName(),
                closestStation->address(),
                closestStation->lat(),
                closestStation->lon(),
                true,
                this
                );
            m_stations.append(searchedStation);
            updateStationSearchStatus(closestStation->stationId(), true);

            // Wycentruj mapę na najbliższej stacji
            m_mapCenter = QGeoCoordinate(closestStation->lat(), closestStation->lon());
            emit mapCenterChanged();

            m_status = QString("Nie znaleziono stacji w %1. Najbliższa stacja znajduje się w %2.").arg(searchedCity, closestStation->cityName());
        } else {
            m_status = QString("Nie znaleziono stacji w %1.").arg(searchedCity);
        }
    } else {
        m_status = QString("Znaleziono %1 stacji w %2.").arg(m_stations.count()).arg(searchedCity);
    }

    emit stationsChanged();
    emit statusChanged();
    reply->deleteLater();
}

/**
 * @brief Handles stations API reply.
 * @param reply Network reply.
 *
 * Processes the response from the GIOŚ API to populate the list of all stations.
 */
void MainWindow::onStationsReply(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        m_status = "Błąd pobierania stacji: " + reply->errorString();
        emit statusChanged();
        reply->deleteLater();
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    QJsonArray stations = doc.array();

    m_allStations.clear();
    for (const QJsonValue &value : stations) {
        QJsonObject obj = value.toObject();
        int id = obj["id"].toInt();
        QString name = obj["stationName"].toString();
        QString city = obj["city"].toObject()["name"].toString();
        QString address = obj["addressStreet"].toString();
        double lat = obj["gegrLat"].toString().toDouble();
        double lon = obj["gegrLon"].toString().toDouble();

        m_allStations.append(new Station(id, name, city, address, lat, lon, false, this));
    }

    emit allStationsChanged();
    reply->deleteLater();
}

/**
 * @brief Handles sensors API reply.
 * @param reply Network reply.
 *
 * Processes the response from the GIOŚ API to populate the list of sensors.
 */
void MainWindow::onSensorsReply(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        m_sensors.clear();
        emit sensorsChanged();
        reply->deleteLater();
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    QJsonArray sensors = doc.array();

    m_sensors.clear();
    for (const QJsonValue &sensorValue : sensors) {
        QJsonObject obj = sensorValue.toObject();
        QJsonObject param = obj["param"].toObject();
        QString paramName = param["paramName"].toString();
        int sensorId = obj["id"].toInt();
        QVariantMap sensorInfo;
        sensorInfo["paramName"] = paramName;
        sensorInfo["sensorId"] = sensorId;
        m_sensors.append(sensorInfo);
    }

    emit sensorsChanged();
    reply->deleteLater();
}

/**
 * @brief Handles sensor data API reply.
 * @param reply Network reply.
 * @param sensorId Sensor ID.
 *
 * Processes the response from the GIOŚ API to store sensor data.
 */
void MainWindow::onSensorDataReply(QNetworkReply *reply, int sensorId)
{
    if (reply->error() != QNetworkReply::NoError) {
        m_sensorData.remove(QString::number(sensorId));
        emit sensorDataChanged();
        reply->deleteLater();
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    QJsonObject obj = doc.object();
    QJsonArray values = obj["values"].toArray();

    QVariantList sensorDataList;
    for (const QJsonValue &value : values) {
        QJsonObject dataPoint = value.toObject();
        QString date = dataPoint["date"].toString();
        QVariant dataValue = dataPoint["value"].toVariant();
        QVariantMap data;
        data["date"] = date;
        data["value"] = dataValue;
        sensorDataList.append(data);
    }

    qDebug() << "Sensor ID:" << sensorId << "Data points:" << sensorDataList.size();
    m_sensorData[QString::number(sensorId)] = sensorDataList;
    emit sensorDataChanged();
    reply->deleteLater();
}
