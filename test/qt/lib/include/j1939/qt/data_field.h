#pragma once

#include <QObject>

#include <j1939/data_field.h>

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
#endif

public:
    DataField(QObject* parent = nullptr) :
        QObject(parent)
    {}

    template <pgns pgn, class Container>
    void populate(const embr::j1939::data_field<pgn, Container>& v)
    {
#if __cpp_fold_expressions
        using traits = pgn::traits<pgn>;
        // Waiting on estd 0.7.3 fix for estd::variadic::values
        //populate(traits::spns{});
#endif
    }
};

}}
