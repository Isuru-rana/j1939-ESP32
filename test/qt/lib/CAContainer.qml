import QtQuick
import QtQuick.Controls

import j1939.cs

Item {
    property Network network
    property int address: network != null ? network.address : 254
    property bool hasAddress: network?.isClaimed

    Label { text: "Addr: " + (hasAddress ? address.toString(16) : "N/A") }
}
