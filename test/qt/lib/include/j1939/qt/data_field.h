#pragma once

#include <QObject>
#include <QVariant>

#include <j1939/data_field.h>
#include <j1939/internal/decompose.h>

namespace embr::j1939::qt { inline namespace v0 {

// EXPERIMENTAL
// Auto population of PGN data field
class DataField : public QObject
{
    Q_OBJECT

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

    template <class T, spns spn>
    void operator()(const T& value, j1939::spn::traits<spn>)
    {
        using traits = spn::traits<spn>;
        QVariant v(value);

        setProperty(traits::name(), v);
    }

#endif

public:
    DataField(QObject* parent = nullptr) :
        QObject(parent)
    {}

    template <pgns pgn, class Container>
    void populate(const embr::j1939::data_field<pgn, Container>& v)
    {
#if __cpp_fold_expressions
        decompose(v, *this);
#endif
    }
};

}}
