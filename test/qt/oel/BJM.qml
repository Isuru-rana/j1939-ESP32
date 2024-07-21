import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import j1939.cs 1.0
import j1939.ca 1.0

Item {
    property Generic generic
    property BJM ca
    property int group: 0

    RowLayout {
        anchors.fill: parent

        Button {
            text: "bjm1.1"
            onPressed: ca.buttonPress(group, 0, true)
            onReleased: ca.buttonPress(group, 0, false)
        }
        Button {
            text: "bjm1.2"
            onPressed: ca.buttonPress(group, 1, true)
            onReleased: ca.buttonPress(group, 1, false)
        }

        Rectangle {
            // DEBT: These layout things aren't working, presumable there's a glitch in
            // CAContainer's anchor.fill procedure
            //Layout.minimumHeight: 50
            //Layout.fillHeight: true
            height: 40
            width: height

            border.color: "gray"
            border.width: 1

            MouseArea {
                anchors.fill: parent
                onPositionChanged: (mouse) => {
                    //ca.updateAxis(group, 0, 0)
                }

                onPressed: (mouse) => {
                    var x = 200 * (mouse.x / width) - 100
                    var y = 200 * (mouse.y / height) - 100
                    console.log("onPressed: ", x, y)
                    ca.updateAxis(group, x, y)
                }
            }
        }
    }

    Connections {
        target: generic

        function onPduReceived(pdu) {
            if(pdu.pgn === 0xfdd6) {
                // TODO: For axis, may be more intuitive to handle that at the C++ CA level
            }
        }
    }
}
