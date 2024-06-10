#pragma once

#include <QObject>
#include <QMetaEnum>

#include <j1939/pdu/header.h>
#include <j1939/pdu.h>

#include <j1939/internal/dispatcher/dispatch.hpp>

#include "data_field.h"

namespace embr::j1939 {

struct pgn_to_string_functor
{
    template <pgns pgn>
    constexpr const char* operator()(internal::in_place_pgn<pgn>)
    {
        return internal::traits_wrapper<pgn>::name();
    }

    constexpr const char* operator()() const { return nullptr; }
};

constexpr const char* to_string(pgns pgn)
{
    return internal::dispatch(pgn_to_string_functor{}, pgn);
}

}

namespace embr::j1939::qt { inline namespace v1 {

// No love
Q_NAMESPACE
//using pgns = embr::j1939::pgns;
enum pgns2
{
    address_claimed = 0xEE
};

Q_ENUM_NS(pgns2)

class Pdu : public QObject
{
    can_id can_id_;
    DataField data_field_;

    //using pgns = pgns2;

    Q_OBJECT

    Q_PROPERTY(pgns pgn READ pgn CONSTANT)
    Q_PROPERTY(uint8_t source_address READ source_address CONSTANT)
    Q_PROPERTY(uint8_t destination_address READ destination_address CONSTANT)
    Q_PROPERTY(DataField* payload READ payload CONSTANT)
    //Q_PROPERTY(uint8_t destination_address READ source_address CONSTANT)

public:
    Pdu(uint32_t id, QObject* parent = nullptr) :
        QObject(parent),
        can_id_{id},
        data_field_(parent)
    {}

    pgns pgn() const
    {
        if(can_id_.is_pdu1())
        {
            pdu1_header h(can_id_);

            return (pgns)h.range();
        }
        else
        {
            pdu2_header h(can_id_);

            return (pgns)h.range();
        }
    }

    uint8_t source_address() const
    {
        return can_id_.source_address();
    }

    uint8_t destination_address() const
    {
        if(can_id_.is_pdu1() == false)   return addresses::null;

        pdu1_header h(can_id_);

        return h.destination_address();
    }

    DataField* payload() { return &data_field_; }

    Q_INVOKABLE QString toString() const
    {
        QString s = QString("pgn=%1 (%2) sa=%3").
            arg(to_string(pgn())).
            arg(int(pgn()), 0, 16).
            arg(int(source_address()), 0, 16);

        // Rather agitating we cannot get QMetaEnum to work comfortably without touching
        // pgn/enum itself.  Fortunately, type traits may be able to help us here
        //auto pgn2 = QMetaEnum::fromType<pgns>().valueToKey(int(pgn()));

        return s;
    }
};

}}
