import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import j1939.ca 1.0

Item {
    property OEL ca

    RowLayout {
        Button {
            text: "left"
            onClicked: ca.leftSignal()
        }

        Button {
            text: "right"
            onClicked: ca.rightSignal()
        }
    }
}
