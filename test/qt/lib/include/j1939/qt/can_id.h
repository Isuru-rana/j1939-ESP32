#pragma once

#include <QObject>

#include <j1939/can_id.h>

namespace embr::j1939::qt { inline namespace v1 {

class CanId : public can_id
{
    using addr_type = uint8_t;

    Q_GADGET

    Q_PROPERTY(pgns pgn READ pgn WRITE setPgn)
    Q_PROPERTY(unsigned priority READ priority)
    Q_PROPERTY(addr_type sourceAddress READ source_address)
    Q_PROPERTY(bool pdu1 READ is_pdu1)

    Q_PROPERTY(addr_type destinationAddress READ pdu_specific)

public:
    constexpr CanId() : can_id{0}   {}
    constexpr explicit CanId(const uint32_t id) : can_id{id}  {}
    constexpr explicit CanId(const can_id& id) : can_id{id.raw()}  {}

    constexpr pgns pgn() const
    {
        return internal::get_pgn(*this);
    }

    // UNTESTED
    void setPgn(pgns pgn)
    {
        if(internal::is_pdu1(pgn))
            range_pdu1(uint16_t(pgn));
        else
            range_pdu2(uint32_t(pgn));
    }

    constexpr unsigned priority() const
    {
        return can_id::priority().value();
    }

    const can_id& raw() const { return *this; }

    // TODO: See if QML duck-type finds this guy like it does with a QObject
    Q_INVOKABLE QString toString() const
    {
        return {};
    }
};

}}
