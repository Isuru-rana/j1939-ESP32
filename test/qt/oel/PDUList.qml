import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import j1939 1.0
import j1939.cs 1.0 as CS
import j1939.ui 1.0 as UI

Item {
    property CS.Generic generic

    ListView {
        id: listView
        model: Session.frameLog
        anchors.fill: parent

        delegate: UI.PDU {
            pdu: modelData
        }
    }

    Connections {
        target: Session

        function onFrameLogChanged(pdu) {
            //idPdu.pdu = pdu
            // DEBT: Ultra clunky since QList doesn't do updates
            listView.model = Session.frameLog
            listView.positionViewAtEnd()
        }
    }
}
