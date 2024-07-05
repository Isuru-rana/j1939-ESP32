import QtQuick
import QtQuick.Controls
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

    Connections {
        target: Session.tp

        function onPacketReceived(id, payload) {
            console.log("tp recv:", to_string_canid(id), payload)
        }
    }

    ColumnLayout {

        anchors.fill: parent

        RowLayout {
            Layout.fillWidth: true
            //Layout.fillHeight: true
            Layout.maximumHeight: 50

            CAContainer {
                Layout.fillWidth: true
                //Layout.fillHeight: true
                network: Session.network
            }

            Button {
                //Layout.fillWidth: true
                //Layout.fillHeight: true
                text: "software_id"
                onClicked: {
                    // software id
                    var software_id = 0xFEDA;
                    var payload = "0123456789ABCDEF";
                    //Session.tp.broadcast(0, software_id, payload);

                    // Coming along
                    Session.tp.listen(0x78);
                    Session.tp.send(0x77, 0x78, software_id, payload);
                }
            }
        }

        Debug1 {
            generic: Session.generic
            ca: Session.clients[0]
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        CCVS {
            generic: Session.generic
            ca: Session.clients[2]
            Layout.fillWidth: true
            Layout.fillHeight: true

        }

        PDUList {
            Layout.fillWidth: true
            Layout.minimumHeight: 200
            Layout.maximumHeight: 200
            generic: Session.generic
        }
    }
}
