#ifndef J1939LIB_H
#define J1939LIB_H

#include <QtQuick/QQuickPaintedItem>

class J1939Lib : public QQuickPaintedItem
{
    Q_OBJECT
    QML_ELEMENT
    Q_DISABLE_COPY(J1939Lib)
public:
    explicit J1939Lib(QQuickItem *parent = nullptr);
    void paint(QPainter *painter) override;
    ~J1939Lib() override;
};

#endif // J1939LIB_H
