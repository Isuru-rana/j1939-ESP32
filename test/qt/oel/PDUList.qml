import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import j1939.cs 1.0 as CS
import j1939.ui 1.0 as UI

Item {
    property CS.Generic generic

    UI.PDU {
        anchors.fill: parent
        id: idPdu
    }

    Connections {
        target: generic

        function onPduReceived(pdu) {
            idPdu.pdu = pdu
        }
    }
}
