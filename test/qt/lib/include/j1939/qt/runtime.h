#pragma once

#include <QHash>
#include <QObject>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQmlComponent>

namespace embr::j1939::qt { inline namespace v1 {

class QmlFactory : public QObject
{
    QHash<const QMetaObject*, QQmlComponent*> mapping_;
    QQmlEngine* engine_;

    Q_OBJECT

public:
    QmlFactory(QQmlEngine* parent) :
        QObject(parent),
        engine_(parent)
    {}

    Q_INVOKABLE QQuickItem* create(const QObject*);

    void map(const QMetaObject* key, const QString& qmlFile);
};

class Runtime : public QObject
{
    QmlFactory caQmlFactory_;

    Q_OBJECT

public:
    // DEBT: Would be prudent to have a non QQmlEngine flavor also
    Runtime(QQmlEngine* parent = nullptr) :
        QObject(parent),
        caQmlFactory_(parent)
    {}

    QmlFactory* caQmlFactory() { return &caQmlFactory_; }
};

}}
