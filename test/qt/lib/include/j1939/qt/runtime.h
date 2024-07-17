#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQmlComponent>

namespace embr::j1939::qt { inline namespace v1 {

class QmlFactory : public QObject
{
    QQmlEngine* engine_;

    Q_OBJECT

public:
    QmlFactory(QQmlEngine* parent = nullptr) : QObject(parent) {}

    Q_INVOKABLE QQuickItem* create();
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
