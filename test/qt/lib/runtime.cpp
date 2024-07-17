#include <QQmlContext>

#include "j1939/qt/runtime.h"

// Guidance from
// https://stackoverflow.com/questions/66618613/qml-c-classes-with-bring-your-own-component

namespace embr::j1939::qt { inline namespace v1 {

QQuickItem* QmlFactory::create(const QObject* o)
{
    const QMetaObject* meta = o->metaObject();
    auto i = mapping_.find(meta);

    if(i != mapping_.end())
    {
        QQmlContext* context = new QQmlContext(engine_);
        // DEBT: Explore 'beginCreate' here
        QObject* created = i.value()->create(context);
        auto item = dynamic_cast<QQuickItem*>(created);
        return item;
    }

    return nullptr;
    //return {};
}

void QmlFactory::map(const QMetaObject* key, const QString& qmlFile)
{
    // DEBT: Does this auto-free if reassigned?
    QUrl url = QUrl::fromLocalFile(qmlFile);
    auto component = new QQmlComponent(engine_, url, this);
    mapping_[key] = component;
}

}}
