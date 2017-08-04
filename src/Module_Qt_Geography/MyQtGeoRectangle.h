#ifndef MYQTGEORECTANGLE_H
#define MYQTGEORECTANGLE_H

#include <QGeoRectangle>
#include <QObject>
#include "MyQtGeoShapeBase.h"

class MyQtGeoRectangle: public MyQtGeoShapeBase
{
public:
    explicit MyQtGeoRectangle(const QGeoCoordinate &bottomLeft, const QGeoCoordinate &topRight, bool *ok,
                              qint32 geoShapeObjectID, QString name, QObject *parent = 0);
    virtual ~MyQtGeoRectangle() {}
     virtual bool containsPoint(QGeoCoordinate geoCoordinate, Qt::FillRule fillRule=Qt::OddEvenFill) ;
    virtual Enum_MyQtGeoShapeType getGeoShapeType() const;

    QGeoRectangle getGeoRectangle() const;
    void setGeoRectangle(const QGeoRectangle &value);

protected:
    QGeoRectangle geoRectangle;
};

#endif // MYQTGEORECTANGLE_H
