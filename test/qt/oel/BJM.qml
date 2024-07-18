import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import j1939.cs 1.0
import j1939.ca 1.0

Item {
    property Generic generic
    property BJM ca

    RowLayout {
        anchors.fill: parent

        Button {
            text: "bjm1.1"
            onPressed: ca.buttonPress(0, 0, true)
            onReleased: ca.buttonPress(0, 0, false)
        }
        Button {
            text: "bjm1.2"
        }
    }
}
