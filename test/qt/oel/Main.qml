import QtQuick

import j1939 1.0
import j1939.cs 1.0

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("Hello World")

    Connections {
        target: Session.generic

        function onPduReceived(pdu) {
            console.log(pdu.pgn)
        }
    }
}
