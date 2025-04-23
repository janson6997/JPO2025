import QtQuick 2.15
import QtQuick.Controls 2.15

Window {
    id: dialog
    modality: Qt.ApplicationModal
    title: "Dane archiwalne"
    width: 600
    height: 400
    visible: false

    Rectangle {
        id: header
        width: parent.width
        height: 50
        color: "#4CAF50"

        Text {
            anchors.centerIn: parent
            text: "Wybierz zapisane dane"
            color: "white"
            font.pixelSize: 20
            font.bold: true
        }
    }

    ListView {
        id: archivedList
        anchors.top: header.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 10
        spacing: 10
        clip: true
        model: mainWindow.archivedStations

        delegate: Rectangle {
            width: parent.width
            height: 80
            color: mouseArea.containsMouse ? "#e0e0e0" : "#f0f0f0"
            radius: 5

            MouseArea {
                id: mouseArea
                anchors.fill: parent
                hoverEnabled: true
                onClicked: {
                    mainWindow.loadArchivedStationData(modelData.stationId, modelData.saveDate)
                    var component = Qt.createComponent("qrc:/StationDialog.qml")
                    if (component.status === Component.Ready) {
                        var address = modelData.address ? modelData.address : "Brak danych"
                        var street = address === "Brak danych" ? "Brak danych" : address.split(" ")[0]
                        var number = address === "Brak danych" ? "" : (address.split(" ").length > 1 ? address.split(" ")[1] : "")
                        var dialog = component.createObject(root, {
                            "stationId": modelData.stationId,
                            "cityName": modelData.cityName,
                            "street": street,
                            "number": number,
                            "saveDate": modelData.saveDate
                        })
                        dialog.open()
                    }
                    dialog.close()
                }
            }

            Column {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 5

                Text {
                    text: "<b>ID stacji:</b> " + modelData.stationId
                    font.pixelSize: 14
                }
                Text {
                    text: "<b>Miasto:</b> " + modelData.cityName
                    font.pixelSize: 14
                }
                Text {
                    text: "<b>Adres:</b> " + (modelData.address ? modelData.address : "Brak danych")
                    font.pixelSize: 14
                }
                Text {
                    text: "<b>Data zapisu:</b> " + modelData.saveDate
                    font.pixelSize: 14
                }
            }
        }
    }

    function open() {
        visible = true
    }

    function close() {
        visible = false
    }
}
