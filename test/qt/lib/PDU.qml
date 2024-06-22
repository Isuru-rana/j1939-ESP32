import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import j1939 1.0 as Lib

// Some ideas from https://wiki.qt.io/How_to_create_columns_in_a_QML_ListView

RowLayout {
    property Lib.pdu pdu
    property variant columnWidths

    Label {
        text: pdu.priority
    }

    Label {
        text: pdu.pgn
    }

    Label {
        text: pdu.source_address
    }

    Label {
        text: pdu.destination_address
    }
}
