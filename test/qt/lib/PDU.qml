import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import j1939 1.0 as Lib

// Some ideas from https://wiki.qt.io/How_to_create_columns_in_a_QML_ListView

RowLayout {
    property Lib.Pdu pdu
    property variant columnWidths

    Label {
        text: pdu.priority
    }

    Label {
        text: to_string(pdu.pgn) + ' (' + pdu.pgn.toString(16).toUpperCase() + ')'
    }

    Label {
        text: pdu.source_address.toString(16).toUpperCase()
    }

    Label {
        text: pdu.destination_address.toString(16).toUpperCase()
    }

    ListView {
        model: pdu.payload.keys()

        delegate: Label {
            text: modelData
        }
    }
}
