#pragma once

#include <QObject>
#include <QQmlPropertyMap>
#include <QVariant>

#include <j1939/pgn/traits.h>
#include <j1939/data_field.h>
#include <j1939/internal/decompose.h>

namespace embr::j1939::qt { inline namespace v0 {

// EXPERIMENTAL
// Auto population of PGN data field
// TODO: setProperty & friends aren't visible from QML (wow)
// See https://stackoverflow.com/questions/34379524/how-to-dynamically-add-remove-qml-properties-inside-c
// Need https://doc.qt.io/qt-6.5/qqmlpropertymap.html
class DataField : public QObject
{
    QQmlPropertyMap map_;
    QQmlPropertyMap name_to_short_name_;

    // DEBT: Use c++20 concept for 'traits'
    template <class traits, typename T>
    void set(const T& v)
    {
        // DEBT: In the end we don't want a scenario where name is null at all here
        if constexpr(traits::name() != nullptr)
            map_[traits::name()] = v;

        name_to_short_name_[traits::name()] = traits::short_name() == nullptr ?
            traits::name() :
            traits::short_name();
    }

    Q_OBJECT

    // DEBT - this guy is dormant I think, document or get rid of him
#if __cpp_fold_expressions
    template <spns spn>
    void populate(spn::traits<spn>)
    {
        using traits = spn::traits<spn>;

        setProperty(traits::name(), 0);
    }

    template <spns... s>
    void populate(estd::variadic::values<spns, s...>)
    {
        (... && populate(spn::traits<s>{}));
    }

#endif

    template <class Rep, class Period, class Tag, class F = estd::internal::units::passthrough<Rep>>
    using unit = estd::internal::units::unit_base<Rep, Period, Tag, F>;

    Q_PROPERTY(QQmlPropertyMap* map READ map CONSTANT)

public:
    DataField(QObject* parent = nullptr) :
        QObject(parent)
    {}

    QQmlPropertyMap* map() { return &map_; }

    QString short_name(const QString& s) const
    {
        return name_to_short_name_[s].toString();
    }

    template <class Rep, class Period, class Tag, class F, spns spn>
    void operator()(j1939::spn::traits<spn>, const unit<Rep, Period, Tag, F>& value)
    {
        using traits = spn::traits<spn>;
        unit<double, estd::ratio<1>, Tag> converted(value);

        QVariant v(converted.count());

        // DEBT: Whole thing was set up to use setProperty, but QML can't see it.
        // a little clunky now since we're doing both and I just tossed map in there
        setProperty(traits::name(), v);
        set<traits>(v);
    }

    template <class T, spns spn>
    void operator()(j1939::spn::traits<spn>, const T& value)
    {
        using traits = spn::traits<spn>;
        // DEBT: Do a special enum variety
        auto v2 = int(value);
        QVariant v(v2);

        setProperty(traits::name(), v);
        set<traits>(v);
    }

    template <pgns pgn, class Container>
    void populate(const embr::j1939::data_field<pgn, Container>& v)
    {
#if __cpp_fold_expressions

#if FEATURE_EMBR_J1939_NO_TRAITS_WRAPPER
        using traits = j1939::pgn::traits<pgn>;
        if constexpr(traits::is_specialized)   decompose(v, *this);
#else
        using traits = j1939::internal::traits_wrapper<pgn>;
        if constexpr(traits::specialized)   decompose(v, *this);
#endif

#endif
    }
};

}}
