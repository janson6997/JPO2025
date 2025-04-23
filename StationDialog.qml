import QtQuick 2.15
import QtQuick.Controls 2.15

Window {
    id: dialog
    modality: Qt.ApplicationModal
    title: "Informacje o stacji"
    visibility: Window.Maximized
    minimumWidth: 700
    minimumHeight: 550
    visible: false

    property int stationId: 0
    property string cityName: ""
    property string street: ""
    property string number: ""
    property string saveDate: ""

    property var selectedSensors: ({})
    property var colors: ["#4CAF50", "#FF0000", "#0000FF", "#FFA500", "#800080", "#00CED1"]

    Rectangle {
        id: header
        width: parent.width
        height: 50
        color: "#4CAF50"

        Text {
            anchors.centerIn: parent
            text: "Stacja nr " + stationId + ", " + cityName + ", ul. " + street + (number ? " " + number : "")
            color: "white"
            font.pixelSize: 20
            font.bold: true
            width: parent.width - 20
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
        }
    }

    Row {
        anchors.top: header.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 10
        spacing: 10

        Column {
            width: parent.width * 0.4
            height: parent.height
            spacing: 10

            Text {
                id: paramsHeader
                width: parent.width
                text: "Wybierz mierzony parametr:"
                font.pixelSize: 16
                color: "#333"
                wrapMode: Text.WordWrap
            }

            ListView {
                id: paramsList
                width: parent.width
                height: parent.height - paramsHeader.height - parent.spacing
                spacing: 10
                clip: true
                model: mainWindow.sensors

                delegate: Rectangle {
                    width: parent.width
                    height: 40
                    color: checkBox.checked ? "#C8E6C9" : (mouseArea.containsMouse ? "#e0e0e0" : "#f0f0f0")
                    radius: 5

                    Row {
                        anchors.left: parent.left
                        anchors.leftMargin: 10
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 5

                        CheckBox {
                            id: checkBox
                            text: modelData.paramName
                            font.pixelSize: 14
                            onCheckedChanged: {
                                if (checked) {
                                    selectedSensors[modelData.sensorId] = modelData.paramName
                                    mainWindow.fetchSensorData(modelData.sensorId)
                                } else {
                                    delete selectedSensors[modelData.sensorId]
                                    mainWindow.removeSensorData(modelData.sensorId)
                                }
                                chartCanvas.requestPaint()
                            }
                        }

                        Rectangle {
                            visible: checkBox.checked
                            width: 12
                            height: 12
                            radius: 6
                            anchors.verticalCenter: parent.verticalCenter
                            color: {
                                var sensorId = modelData.sensorId
                                var selectedIds = Object.keys(selectedSensors)
                                var index = selectedIds.indexOf(sensorId.toString())
                                return index >= 0 && index < colors.length ? colors[index] : "transparent"
                            }
                        }
                    }

                    MouseArea {
                        id: mouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: {
                            checkBox.checked = !checkBox.checked
                        }
                    }
                }
            }
        }

        Column {
            width: parent.width * 0.6
            height: parent.height
            spacing: 10

            Rectangle {
                width: parent.width
                height: parent.height * 0.6
                color: "#f0f0f0"
                border.color: "black"
                border.width: 1
                anchors.rightMargin: 20

                Canvas {
                    id: chartCanvas
                    anchors.fill: parent
                    anchors.margins: 50

                    onPaint: {
                        var ctx = getContext("2d")
                        ctx.clearRect(0, 0, width, height)
                        ctx.strokeStyle = "black"
                        ctx.lineWidth = 2
                        ctx.font = "14px Arial"

                        ctx.strokeStyle = "#d3d3d3"
                        ctx.lineWidth = 1
                        for (var x = 0; x <= width; x += width / 10) {
                            ctx.beginPath()
                            ctx.moveTo(x, 0)
                            ctx.lineTo(x, height)
                            ctx.stroke()
                        }
                        for (var y = 0; y <= height; y += height / 10) {
                            ctx.beginPath()
                            ctx.moveTo(0, y)
                            ctx.lineTo(width, y)
                            ctx.stroke()
                        }

                        ctx.strokeStyle = "black"
                        ctx.lineWidth = 2
                        ctx.beginPath()
                        ctx.moveTo(0, height)
                        ctx.lineTo(width, height)
                        ctx.moveTo(0, 0)
                        ctx.lineTo(0, height)
                        ctx.stroke()

                        var sensorData = mainWindow.sensorData
                        var selectedSensorIds = Object.keys(selectedSensors)
                        if (selectedSensorIds.length === 0 || Object.keys(sensorData).length === 0) {
                            ctx.fillText("Wybierz mierzone parametry aby wyświetlić odczyty.", width / 2 - 150, height / 2)
                            return
                        }

                        var globalMaxValue = -Number.MAX_VALUE
                        var globalMinValue = Number.MAX_VALUE
                        var allDataPoints = []
                        var timePoints = []

                        for (var i = 0; i < selectedSensorIds.length; i++) {
                            var sensorId = selectedSensorIds[i]
                            var data = sensorData[sensorId]
                            if (!data) {
                                console.log("No data for sensor:", sensorId)
                                continue
                            }

                            for (var j = 0; j < data.length; j++) {
                                var value = data[j].value
                                if (value !== null && !isNaN(value)) {
                                    globalMaxValue = Math.max(globalMaxValue, value)
                                    globalMinValue = Math.min(globalMinValue, value)
                                }
                                if (i === 0) {
                                    timePoints.push(data[j].date)
                                }
                            }
                            allDataPoints.push(data)
                        }

                        console.log("Time points:", timePoints.length, "Min value:", globalMinValue, "Max value:", globalMaxValue, "Selected sensors:", selectedSensorIds)

                        if (globalMaxValue === globalMinValue) {
                            globalMaxValue += 1
                            globalMinValue -= 1
                        }
                        if (timePoints.length === 0) {
                            ctx.fillText("Brak danych czasowych.", width / 2 - 100, height / 2)
                            return
                        }

                        ctx.strokeStyle = "black"
                        ctx.lineWidth = 1
                        ctx.setLineDash([5, 5])
                        for (var i = 0; i < timePoints.length; i++) {
                            var time = timePoints[i].split(" ")[1]
                            if (time === "00:00:00") {
                                var x = (i / (timePoints.length - 1)) * width
                                ctx.beginPath()
                                ctx.moveTo(x, 0)
                                ctx.lineTo(x, height)
                                ctx.stroke()
                            }
                        }
                        ctx.setLineDash([])

                        for (var s = 0; s < selectedSensorIds.length && s < colors.length; s++) {
                            var sensorId = selectedSensorIds[s]
                            var data = sensorData[sensorId]
                            if (!data) continue

                            ctx.strokeStyle = colors[s]
                            ctx.lineWidth = 2
                            ctx.beginPath()
                            var firstPoint = true
                            for (var i = 0; i < data.length; i++) {
                                var value = data[i].value
                                if (value === null || isNaN(value)) continue
                                var x = (i / (data.length - 1)) * width
                                var y = height - ((value - globalMinValue) / (globalMaxValue - globalMinValue)) * height
                                if (firstPoint) {
                                    ctx.moveTo(x, y)
                                    firstPoint = false
                                } else {
                                    ctx.lineTo(x, y)
                                }
                            }
                            ctx.stroke()
                        }

                        ctx.fillStyle = "black"
                        var lastHour = -1
                        for (var i = 0; i < timePoints.length; i++) {
                            var dateTime = timePoints[i].split(" ")
                            var date = dateTime[0].split("-")
                            var time = dateTime[1]
                            var hour = parseInt(time.split(":")[0])
                            var minutes = time.split(":")[1]

                            if (hour % 4 === 0 && hour !== lastHour) {
                                var x = (i / (timePoints.length - 1)) * width
                                var timeLabel = (hour < 10 ? "0" + hour : hour) + ":" + minutes
                                var dateLabel = date[2] + "." + date[1] + "." + date[0].slice(2)
                                ctx.save()
                                ctx.translate(x, height - 10)
                                ctx.rotate(-Math.PI / 4)
                                ctx.fillText(timeLabel, 0, 0)
                                ctx.restore()
                                ctx.fillText(dateLabel, x - 20, height - 5)
                                lastHour = hour
                            }
                        }

                        ctx.fillStyle = "black"
                        for (var i = 0; i <= 5; i++) {
                            var value = globalMinValue + (i * (globalMaxValue - globalMinValue) / 5)
                            var y = height - (i * height / 5)
                            if (!isNaN(value)) {
                                ctx.fillText(value.toFixed(1), +10, y - 5)
                            }
                        }

                        ctx.fillStyle = "black"
                        ctx.fillText("Czas", width / 2, height + 60)
                        ctx.save()
                        ctx.translate(30, height / 2)
                        ctx.rotate(-Math.PI / 2)
                        ctx.fillText("Wartość (µg/m³)", 0, 0)
                        ctx.restore()
                    }
                }
            }

            Rectangle {
                width: parent.width
                height: parent.height * 0.3
                color: "#f0f0f0"
                border.color: "black"
                border.width: 1
                anchors.rightMargin: 20

                Column {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 10

                    Rectangle {
                        width: parent.width
                        height: 30
                        color: "#4CAF50"

                        Text {
                            anchors.centerIn: parent
                            text: "Wyświetl szczegółowe dane:"
                            color: "white"
                            font.pixelSize: 16
                            font.bold: true
                        }
                    }

                    ComboBox {
                        id: paramSelector
                        width: parent.width
                        height: 40
                        model: mainWindow.sensors
                        textRole: "paramName"
                        font.pixelSize: 14
                        onCurrentIndexChanged: {
                            if (currentIndex >= 0) {
                                var sensorId = mainWindow.sensors[currentIndex].sensorId
                                mainWindow.fetchSensorData(sensorId)
                            }
                        }
                    }

                    Button {
                        id: saveButton
                        width: parent.width
                        height: 40
                        text: "Zapisz dane"
                        font.pixelSize: 14
                        onClicked: {
                            mainWindow.saveStationData(stationId, cityName, street + (number ? " " + number : ""))
                        }
                    }

                    Column {
                        width: parent.width
                        spacing: 5
                        visible: paramSelector.currentIndex >= 0

                        Text {
                            id: latestValueText
                            width: parent.width
                            font.pixelSize: 14
                            color: "#4CAF50"
                            wrapMode: Text.WordWrap
                            text: {
                                if (paramSelector.currentIndex < 0) return ""
                                var sensorId = mainWindow.sensors[paramSelector.currentIndex].sensorId
                                var data = mainWindow.sensorData[sensorId]
                                if (!data || data.length === 0) return "Brak danych"
                                var latest = data[0]
                                return "Aktualny odczyt: " + (latest.value !== null ? latest.value.toFixed(2) : "Brak") + " µg/m³ (" + latest.date + ")"
                            }
                        }

                        Text {
                            width: parent.width
                            font.pixelSize: 14
                            wrapMode: Text.WordWrap
                            text: {
                                if (paramSelector.currentIndex < 0) return ""
                                var sensorId = mainWindow.sensors[paramSelector.currentIndex].sensorId
                                var data = mainWindow.sensorData[sensorId]
                                if (!data || data.length === 0) return "Średnia wartość: Brak danych"
                                var sum = 0
                                var count = 0
                                for (var i = 0; i < data.length; i++) {
                                    if (data[i].value !== null && !isNaN(data[i].value)) {
                                        sum += data[i].value
                                        count++
                                    }
                                }
                                var avg = count > 0 ? (sum / count).toFixed(2) : "Brak"
                                return "Średnia wartość: " + avg + " µg/m³"
                            }
                        }

                        Text {
                            width: parent.width
                            font.pixelSize: 14
                            wrapMode: Text.WordWrap
                            text: {
                                if (paramSelector.currentIndex < 0) return ""
                                var sensorId = mainWindow.sensors[paramSelector.currentIndex].sensorId
                                var data = mainWindow.sensorData[sensorId]
                                if (!data || data.length === 0) return "Minimalna wartość: Brak danych"
                                var min = Number.MAX_VALUE
                                for (var i = 0; i < data.length; i++) {
                                    if (data[i].value !== null && !isNaN(data[i].value)) {
                                        min = Math.min(min, data[i].value)
                                    }
                                }
                                return "Minimalna wartość: " + (min !== Number.MAX_VALUE ? min.toFixed(2) : "Brak") + " µg/m³"
                            }
                        }

                        Text {
                            width: parent.width
                            font.pixelSize: 14
                            wrapMode: Text.WordWrap
                            text: {
                                if (paramSelector.currentIndex < 0) return ""
                                var sensorId = mainWindow.sensors[paramSelector.currentIndex].sensorId
                                var data = mainWindow.sensorData[sensorId]
                                if (!data || data.length === 0) return "Maksymalna wartość: Brak danych"
                                var max = -Number.MAX_VALUE
                                for (var i = 0; i < data.length; i++) {
                                    if (data[i].value !== null && !isNaN(data[i].value)) {
                                        max = Math.max(max, data[i].value)
                                    }
                                }
                                return "Maksymalna wartość: " + (max !== -Number.MAX_VALUE ? max.toFixed(2) : "Brak") + " µg/m³"
                            }
                        }
                    }
                }
            }
        }
    }

    Connections {
        target: mainWindow
        function onSensorDataChanged() {
            console.log("Sensor data changed, requesting paint and updating stats")
            chartCanvas.requestPaint()
            latestValueText.text = Qt.binding(function() {
                if (paramSelector.currentIndex < 0) return ""
                var sensorId = mainWindow.sensors[paramSelector.currentIndex].sensorId
                var data = mainWindow.sensorData[sensorId]
                if (!data || data.length === 0) return "Brak danych"
                var latest = data[0]
                return "Aktualny odczyt: " + (latest.value !== null ? latest.value.toFixed(2) : "Brak") + " µg/m³ (" + latest.date + ")"
            })
        }
    }

    function open() {
        visible = true
    }
}
