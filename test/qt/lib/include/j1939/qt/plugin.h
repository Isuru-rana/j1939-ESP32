#pragma once

#include <QObject>

#include "fwd.h"
#include <j1939/NAME/industry_groups.h>
#include <j1939/pgn/enum.h>

namespace embr::j1939::qt { inline namespace v1 {

class Plugin
{
public:
    static void init();
};

// Guidance from
// https://stackoverflow.com/questions/50281239/qt-define-global-function-for-qml

class API : public QObject
{
    Q_OBJECT
public:
    explicit API(QObject* parent) : QObject(parent) {}

    Q_INVOKABLE QString to_string(pgns, bool abbrev);

    // Dummies just to test overloading
    Q_INVOKABLE QString to_string(industry_groups)  { return "ig N/A"; }
    // 'int' flavor seems to greedily consume everything
    //Q_INVOKABLE QString to_string(int)  { return "N/A!"; }
};

}}
