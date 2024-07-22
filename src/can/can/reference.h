#pragma once

#include <assert.h>

#include "fwd.h"

#include <estd/algorithm.h>
#include <estd/span.h>

namespace embr { namespace can {

// This namespace encloses synthetic / reference transport information
// which can be useful for constructing real live ones, or for facilitating
// unit tests
namespace reference {

struct transport
{
    struct frame
    {
        uint32_t id : 29;
        bool extended : 1;
        bool rtr : 1;
        uint8_t dlc;
        uint8_t payload[8];

        frame() = default;

        frame(uint32_t id) :
            extended{false},
            rtr{false},
            dlc(0)
        {}

        frame(uint32_t id, uint8_t dlc, const uint8_t* p, frame_flags f) :
            id{id},
            extended{(f & FRAME_EXT) != 0},
            rtr{(f & FRAME_RTR) != 0},
            dlc{dlc}
        {
            assert(dlc <= 8);

            estd::copy_n(p, dlc, payload);
        }

        frame(const frame&) = default;

        frame(uint32_t can_id, const estd::span<uint8_t>& data) : id{can_id},
            dlc{(uint8_t)data.size()}
        {
            const uint8_t* c = data.begin();

            assert(dlc <= 8);

            estd::copy_n(c, dlc, payload);
        }
    };
};


}

template <>
struct frame_traits<reference::transport::frame>
{
    using frame = reference::transport::frame;

    inline static frame create(uint32_t id)
    {
        return frame(id);
    }

    inline static frame create(uint32_t id, const uint8_t* payload, uint8_t length, frame_flags f)
    {
        return frame(id, length, payload, f);
    }

    constexpr static uint32_t id(const frame& f) { return f.id; }

    static void id(frame& f, uint32_t v) { f.id = v; }

    static void length(frame& f, unsigned v) { f.dlc = v; }

    constexpr static unsigned length(const frame& f) { return f.dlc; }

    static uint8_t* payload(frame& f)
    {
        return f.payload;
    }

    constexpr static const uint8_t* payload(const frame& f)
    {
        return f.payload;
    }

    static void rtr(frame& message, bool) {}        // TODO

    constexpr static bool extended(const frame& f)
    {
        return f.extended;
    }

    static void extended(frame& f, bool is_extended)
    {
        f.extended = is_extended;
    }

    constexpr static bool rtr(const frame& message)
    {
        // TODO
        return {};
    }
};


}}
