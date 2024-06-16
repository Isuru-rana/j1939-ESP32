import QtQuick
import QtQuick.Layouts

import j1939 1.0
import j1939.cs 1.0
import j1939.ui 1.0

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("Hello World")

    Connections {
        target: Session.generic

        function onPduReceived(pdu) {
            // Works great, just done by Debug1 now
            //console.log(pdu)
        }
    }

    ColumnLayout {

        anchors.fill: parent

        CAContainer {
            network: Session.network
            Layout.fillWidth: true
        }

        Debug1 {
            generic: Session.generic
            ca: Session.clients[0]
            Layout.fillWidth: true

        }

        CCVS {
            generic: Session.generic
            ca: Session.clients[2]
            Layout.fillWidth: true

        }
    }
}
