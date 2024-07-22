#include <j1939/internal/dispatcher/incoming2.hpp>

#include <j1939/NAME/vehicle_systems.h>
#include <j1939/NAME/function.h>
#include <j1939/NAME/industry_groups.h>

#include <j1939/pdu.h>
#include <j1939/data_field/bjm1.hpp>

#include "j1939/qt/ca/bjm.h"


namespace embr::j1939::qt::ca { inline namespace v1 {

BJM::BJM(QObject* parent) :
    ControllerApplication(parent)
{
    network_.name().arbitrary_address_capable(true);
    network_.name().industry_group(int(industry_groups::construction));
    network_.name().function_instance(0);
    network_.name().function((int)function_fields::joystick_control);

    network_.setTag("BJM");
}

static void adjust(pdu<pgns::bjm1>& p, unsigned group)
{
    switch(group)
    {
    case 0:
        break;

    case 1:
        p.range(uint32_t(pgns::bjm2));
        break;

    case 2:
        p.range(uint32_t(pgns::bjm3));
        break;

    default:
        break;
    }
}

void BJM::buttonPress(unsigned group, unsigned num, bool down)
{
    pdu<pgns::bjm1> p(network_.address(), null_t{});

    using m = j1939::spn::measured;

    //p.range(p.range() + 1);
    adjust(p, group);

    const m cmd = down ? m::enabled : m::disabled;

    switch(num)
    {
        case 0:
            p.button1_pressed(cmd);
            break;

        case 1:
            p.button2_pressed(cmd);
            break;

        default:
            return;
    }


    send(p);
}

// DEBT: https://github.com/malachi-iot/estdlib/issues/45
// DEBT: Enforce that 'Rep' is signed
template <class Rep, class Period, class F>
constexpr estd::internal::units::unit_base<Rep, Period, F> operator -(
    const estd::internal::units::unit_base<Rep, Period, F>& v)
{
    return estd::internal::units::unit_base<Rep, Period, F>(-v.count());
    //return {-v.count()};
}

template <class Rep, class Period, class F>
void serializeAxis(
    pdu<pgns::bjm1>& p,
    const embr::units::percent<Rep, Period, F>& x,
    const embr::units::percent<Rep, Period, F>& y
    )
{
    using pct = embr::units::percent<Rep, Period, F>;
    using m = j1939::spn::measured;

    if(x.count() < 0)
    {
        p.x_axis_lever_left(m::on);
        p.x_axis_lever_right(m::off);
        p.x_axis_position(pct(-x.count()));
    }
    else
    {
        p.x_axis_lever_left(m::off);
        p.x_axis_lever_right(m::on);
        p.x_axis_position(x);
    }

    if(y.count() < 0)
    {
        p.y_axis_lever_back(m::on);
        p.y_axis_lever_forward(m::off);
        // DEBT: Need a +/- standalone operator for units
        // https://github.com/malachi-iot/estdlib/issues/45
        p.y_axis_position(pct(-y.count()));
    }
    else
    {
        p.y_axis_lever_back(m::off);
        p.y_axis_lever_forward(m::on);
        p.y_axis_position(y);
    }
}

template <class Rep, class Period, class F>
bool deserializeAxis(
    const pdu<pgns::bjm1>& p,
    // DEBT: Naughty, we prefer pointers in this case
    embr::units::percent<Rep, Period, F>& x,
    embr::units::percent<Rep, Period, F>& y
    )
{
    using pct = embr::units::percent<Rep, Period, F>;
    using m = j1939::spn::measured;
    using _pct = unit_type<spns::joystick1_x_axis_position>;
    using traits = spn::traits<spns::joystick1_x_axis_position>;

    const _pct xv = p.x_axis_position();
    const _pct yv = p.y_axis_position();

    if(traits::noop(xv.root_count(), false))    return false;

    y = yv;

    if(p.x_axis_lever_left() == m::on)
    {
        x = -pct(xv);
    }
    else
        x = xv;

    if(p.y_axis_lever_back() == m::on)  y = -y;

    return true;
}

// DEBT: Consider making a non-qt utility function to help with this
void BJM::updateAxis(unsigned group, double x, double y)
{
    pdu<pgns::bjm1> p(network_.address(), null_t{});

    adjust(p, group);

    using m = j1939::spn::measured;
    //using pct = unit_type<spns::joystick1_x_axis_position>;   // wants native units
    using pct = embr::units::percent<double>;

    serializeAxis(p, pct(x), pct(y));

    send(p);
}


void BJM::frameReceived(QCanBusDevice* device, const QCanBusFrame& frame)
{
    transport_type t{device};
    network_.frameReceived(device, frame);
    v2::process_incoming(*this, t, frame);
}


void BJM::start(QCanBusDevice* device)
{
    connect_network(device);
}

auto BJM::process_incoming(transport_type&, const pdu<pgns::bjm1>& p) -> result
{
    embr::units::percent<qreal> x{0}, y{0};

    bool hasAxis = deserializeAxis(p, x, y);

    //qDebug() << "BJM::process incoming x" << x.count() << "y" << y.count();

    if(hasAxis)
        emit axisObserved(0, QPointF(x.count(), y.count()));

    return result::ok();
}

}}
