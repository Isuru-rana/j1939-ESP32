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
            checkable: true
            id: btnLeft
            text: "left"
            onClicked: {
                ca.leftSignal()
                btnRight.checked = false;
            }
            palette {
                id: btnLeftP
            }
        }

        Button {
            checkable: true
            id: btnRight
            text: "right"
            onClicked: {
                ca.rightSignal()
                btnLeft.checked = false
            }
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
                    case 1: btnLeftP.button = "green"; break;
                    case 0: btnLeftP.button = "white"; break;
                }

                switch(pdu.payload.right_turn_signal)
                {
                    case 1: btnRightP.button = "green"; break;
                    case 0: btnRightP.button = "white"; break;
                }
            }
        }
    }
}
