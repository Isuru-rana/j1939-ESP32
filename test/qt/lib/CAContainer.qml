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
    required property Runtime runtime
    required property Generic generic
    property QmlFactory factory: runtime.caQmlFactory

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

        /*
        Label {
            text: modelData.network.tag
        }   */
        Rectangle {
            Layout.minimumWidth: 200
            Layout.minimumHeight: 50
            id: item

            //border.color: "gray"
            //border.width: 1

            Component.onCompleted: {
                var o = factory.create(modelData)

                if(o !== null)
                {
                    o.parent = item
                    o.ca = modelData
                    o.generic = generic
                    o.anchors.fill = item
                }
            }
        }
    }
}
