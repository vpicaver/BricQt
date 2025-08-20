import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("Hello World")

    ColumnLayout {
        anchors.fill:  parent

        RowLayout {

            Text {
                text: connected ? "Connected" : "Not Connected"
            }

            Button {
                text: "Connect"
                onClicked: {
                    bricClient.startScan();
                }
            }
        }

        ScrollView {
        Layout.fillHeight: true

            ListView {
                model: bricModel

                delegate: RowLayout {
                    required property string timestampString
                    required property real   distanceMeters
                    required property real   azimuthDegrees
                    required property real   inclinationDegrees
                    required property real   temperatureCelsius

                    Text { text: timestampString }
                    Text { text: `d=${distanceMeters.toFixed(3)} m` }
                    Text { text: `az=${azimuthDegrees.toFixed(2)}°` }
                    Text { text: `inc=${inclinationDegrees.toFixed(2)}°` }
                    Text { text: `T=${temperatureCelsius.toFixed(1)}°C` }

                }
            }
        }
    }
}
