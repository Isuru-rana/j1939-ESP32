import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import j1939.cs 1.0
import j1939.ca 1.0

Item {
    property Generic generic
    property CM1 ca

    Slider {
        from: 0
        to: 100
        onValueChanged: ca.requestFanSpeed(value)
    }
}
