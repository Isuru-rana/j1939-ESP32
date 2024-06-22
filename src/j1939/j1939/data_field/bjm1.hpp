// Basic Joystick
#pragma once

#include "base.h"

#include "../slots.hpp"

#include "../slots/macro/push.h"

namespace embr { namespace j1939 {

namespace spn {

template <>
struct type_traits<spns::joystick1_x_axis_position> :
    internal::slot_type_traits<slots::SAEpc02>
{
    static constexpr const char* name() { return "x_axis_position"; }
};

template <>
struct type_traits<spns::joystick1_y_axis_position> :
    internal::slot_type_traits<slots::SAEpc02>
{
    static constexpr const char* name() { return "y_axis_position"; }
};

template <>
struct type_traits<spns::joystick1_button1_pressed_status> :
    internal::measured_type_traits
{
    static constexpr const char* name() { return "button1_pressed_status"; }    
};

template <>
struct type_traits<spns::joystick1_button2_pressed_status> :
    internal::measured_type_traits {};

template <>
struct type_traits<spns::joystick1_button3_pressed_status> :
    internal::measured_type_traits {};

template <>
struct type_traits<spns::joystick1_button4_pressed_status> :
    internal::measured_type_traits {};


template<>
constexpr descriptor get_descriptor<spns::joystick1_x_axis_position>()
{
    return { 1, 7, 10 };
}

template<>
constexpr descriptor get_descriptor<spns::joystick1_y_axis_position>()
{
    return { 3, 7, 10 };
}

template<>
constexpr descriptor get_descriptor<spns::joystick1_button1_pressed_status>()
{
    return { 6, 7, 2 };
}

template<>
constexpr descriptor get_descriptor<spns::joystick1_button2_pressed_status>()
{
    return { 6, 5, 2 };
}

template<>
constexpr descriptor get_descriptor<spns::joystick1_button3_pressed_status>()
{
    return { 6, 3, 2 };
}

template<>
constexpr descriptor get_descriptor<spns::joystick1_button4_pressed_status>()
{
    return { 6, 1, 2 };
}

}

namespace pgn {

template <>
struct traits<pgns::basic_joystick_message_1> : internal::traits_base
{
    // DEBT Fix up discrepancy between this and return method flavor.  Take
    // into account which, if either, is more friendly with ROM-based strings
    // (i.e. Arduino's F() macro)
    //static constexpr const char name[] = "Basic Joystick Message 1";
    //static constexpr const char abbrev[] = "BJM1";
    static constexpr const char* name() { return "Basic Joystick Message 1"; }
    static constexpr const char* abbrev() { return "BJM1"; }

    // Extra s::'s do work, but without associated get_definitions all built out
    // sometimes spn::traits gets mad
    using spns = internal::spns_list<
        /*
        s::joystick1_x_axis_neutral_position_status,
        s::joystick1_x_lever_left_neg_pos_status,   */
        s::joystick1_x_axis_position,
        s::joystick1_y_axis_position,
        s::joystick1_button4_pressed_status,
        s::joystick1_button3_pressed_status,
        s::joystick1_button2_pressed_status,
        s::joystick1_button1_pressed_status>;

        /*
        s::joystick1_button8_pressed_status,
        s::joystick1_button7_pressed_status,
        s::joystick1_button6_pressed_status,
        s::joystick1_button5_pressed_status,
        s::joystick1_button12_pressed_status,
        s::joystick1_button11_pressed_status,
        s::joystick1_button10_pressed_status,
        s::joystick1_button9_pressed_status>;   */
};

}


template<class TContainer>
struct data_field<pgns::basic_joystick_message_1, TContainer> :
    internal::data_field_base<TContainer>
{
    typedef internal::data_field_base<TContainer> base_type;

    ESTD_CPP_FORWARDING_CTOR(data_field)

    EMBR_J1939_PROPERTY(joystick1_x_axis_position)
    EMBR_J1939_PROPERTY(joystick1_y_axis_position)

    EMBR_J1939_PROPERTY_ALIAS(joystick1_button1_pressed_status, button1_pressed)
    EMBR_J1939_PROPERTY_ALIAS(joystick1_button2_pressed_status, button2_pressed)
    EMBR_J1939_PROPERTY_ALIAS(joystick1_button1_pressed_status, button3_pressed)
    EMBR_J1939_PROPERTY_ALIAS(joystick1_button2_pressed_status, button4_pressed)
};

namespace internal {

template <class C>
struct payload_put<pgns::basic_joystick_message_1, C> : estd::internal::ostream_functor_tag
{
    const data_field<pgns::basic_joystick_message_1, C>& payload;

    constexpr explicit payload_put(const data_field<pgns::bjm1, C>& payload) :
        payload{payload} {}

    template <class Streambuf, class Base>
    void operator()(estd::detail::basic_ostream<Streambuf, Base>& out) const
    {
        out << "b1=" << (unsigned)payload.button1_pressed();
    }
};

}

}}

#include "../slots/macro/pop.h"
