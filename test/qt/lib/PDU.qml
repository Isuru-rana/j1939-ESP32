import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import j1939 1.0 as Lib

// Some ideas from https://wiki.qt.io/How_to_create_columns_in_a_QML_ListView

RowLayout {
    property Lib.Pdu pdu
    property variant columnWidths
    property variant spns: pdu.payload.keys()

    //onSpnsChanged: console.log("spns:", spns)

    Label {
        text: pdu.priority
    }

    Label {
        text: to_string(pdu.pgn, true) + ' (' + pdu.pgn.toString(16).toUpperCase() + ')'
    }

    Label {
        text: pdu.source_address.toString(16).toUpperCase()
    }

    Label {
        text: pdu.destination_address.toString(16).toUpperCase()
    }

    ListView {
        model: spns
        Layout.minimumWidth: 400
        Layout.minimumHeight: 50

        orientation: ListView.Horizontal
        spacing: 4

        /*
        delegate: Rectangle {
            //border.color: "red"
            //border.width: 2
            //Layout.minimumWidth: 50
            width: 100

            Label {
                anchors.fill: parent
                text: modelData
                horizontalAlignment: Label.Left
            }
        }   */
        delegate: ColumnLayout {

            property string spnName: modelData

            Label {
                Layout.fillWidth: true
                text: pdu.short_name(spnName)
            }

            Label {
                Layout.fillWidth: true
                text: pdu.payload[modelData]
            }
        }
    }
}
