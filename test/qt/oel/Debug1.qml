import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import j1939.cs 1.0
import j1939.ca 1.0

Item {
    property Generic generic
    property OEL ca

    RowLayout {
        Button {
            id: btnLeft
            text: "left"
            onClicked: ca.leftSignal()
            palette {
                id: btnLeftP
            }
        }

        Button {
            id: btnRight
            text: "right"
            onClicked: ca.rightSignal()
            palette {
                id: btnRightP
            }
        }

        Button {
            id: btnHazard
            text: "hazard"
            onClicked: ca.hazardPressed()
            palette {
                id: btnHazardP
            }
        }
    }

    Connections {
        target: generic

        function onPduReceived(pdu) {
            console.log(pdu)
            if(pdu.pgn === 0xFE41)
            {
                //console.log(JSON.stringify(pdu.payload.map))
                //console.log("left=", pdu.payload.left_turn_signal, ", right=", pdu.payload.right_turn_signal)
                //console.log("left=", pdu.payload.map.left_turn_signal, ", right=", pdu.payload.map.right_turn_signal)

                switch(pdu.payload.left_turn_signal)
                {
                    case 0: btnLeftP.button = "green"; break;
                    case 1: btnLeftP.button = "white"; break;
                }

                switch(pdu.payload.right_turn_signal)
                {
                    case 0: btnRightP.button = "green"; break;
                    case 1: btnRightP.button = "white"; break;
                }
            }
        }
    }
}
