import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import j1939
import j1939.cs

// Guidance from
// https://stackoverflow.com/questions/32969414/populate-gridlayout-with-repeater

GridLayout {
    id: root
    // List of 'ControllerApplication'
    property var model

    columns: 2
    flow: GridLayout.TopToBottom
    rows: repeater1.count

    Repeater {
        id: repeater1
        model: root.model

        CADesc {
            network: modelData.network
        }
    }

    Repeater {
        model: root.model

        Label {
            text: modelData.network.tag
        }
    }
}
