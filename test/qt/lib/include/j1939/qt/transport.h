#pragma once

#include <QCanBus>

#include <can/fwd.h>

// DEBT: In a poor location

namespace embr::can {

template <>
struct frame_traits<QCanBusFrame>
{
    using frame = struct QCanBusFrame;

    static frame create(uint32_t id, const uint8_t* payload, unsigned dlc)
    {
        QCanBusFrame f;

        f.setFrameId(id);
        f.setPayload(QByteArray((const char*)payload, dlc));

        // DEBT: Hard wired to extended id for j1939
        f.setExtendedFrameFormat(true);

        return f;
    }

};


struct qt_transport
{
    using frame = QCanBusFrame;

    QCanBusDevice* device_;

    inline bool send(const frame& f)
    {
        return device_->writeFrame(f);
    }
};

}
