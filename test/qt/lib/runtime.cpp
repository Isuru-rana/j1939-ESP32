#include "j1939/qt/runtime.h"

// Guidance from
// https://stackoverflow.com/questions/66618613/qml-c-classes-with-bring-your-own-component

namespace embr::j1939::qt { inline namespace v1 {

QQuickItem* QmlFactory::create()
{
    return nullptr;
    //return {};
}

}}
