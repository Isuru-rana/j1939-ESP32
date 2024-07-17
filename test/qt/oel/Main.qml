import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import j1939 1.0
import j1939.cs 1.0
import j1939.ui 1.0

Window {
    width: 800
    height: 600
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

        CAContainer {
            model: Session.clients
            runtime: Session.runtime
            generic: Session.generic
        }

        RowLayout {
            Layout.fillWidth: true
            //Layout.fillHeight: true
            Layout.maximumHeight: 50

            CADesc {
                Layout.fillWidth: true
                //Layout.fillHeight: true
                // OEL (serviced by Debug1)
                network: Session.clients[0].network
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
            //Layout.fillHeight: true
            Layout.minimumHeight: 50
        }

        BJM {
            generic: Session.generic
            Layout.fillWidth: true
            Layout.minimumHeight: 50
        }

        CCVS {
            generic: Session.generic
            ca: Session.clients[2]
            Layout.fillWidth: true
            //Layout.fillHeight: true
            Layout.minimumHeight: 50

        }

        // Log
        PDUList {
            Layout.fillWidth: true
            Layout.minimumHeight: 250
            Layout.maximumHeight: 300
            Layout.fillHeight: true
            generic: Session.generic
        }
    }
}
