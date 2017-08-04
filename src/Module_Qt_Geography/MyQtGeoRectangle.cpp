#include "MyQtGeoRectangle.h"
#include <QDebug>

MyQtGeoRectangle::MyQtGeoRectangle(const QGeoCoordinate &bottomLeft, const QGeoCoordinate &topRight,
                     bool *ok, qint32 geoShapeObjectID, QString name, QObject *parent)   : MyQtGeoShapeBase(geoShapeObjectID,name,parent)
{
    geoRectangle.setBottomLeft(bottomLeft);
    geoRectangle.setTopRight(topRight);
    *ok=geoRectangle.isValid();
}

bool  MyQtGeoRectangle::containsPoint(QGeoCoordinate geoCoordinate, Qt::FillRule fillRule)
{
    if(!geoCoordinate.isValid())
        qDebug()<<"Error: invalid geoCooridinate:"<<geoCoordinate;
    return geoRectangle.contains(geoCoordinate);
}

Enum_MyQtGeoShapeType MyQtGeoRectangle::getGeoShapeType() const
{
    return EV_MyQtGeoShapeType_Rectangle;
}


QGeoRectangle MyQtGeoRectangle::getGeoRectangle() const
{
    return geoRectangle;
}

void MyQtGeoRectangle::setGeoRectangle(const QGeoRectangle &value)
{
    geoRectangle = value;
}
