import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import j1939.cs 1.0
import j1939.ca 1.0

Item {
    property Generic generic
    property CCVS ca

    RowLayout {
        Button {
            text: "brakes"
            onPressed: ca.brakeSwitchPressed()
            onReleased: ca.brakeSwitchReleased()
        }
    }
}
