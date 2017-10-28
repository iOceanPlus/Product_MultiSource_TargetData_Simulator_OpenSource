#include "MyQtGeoShapeBase.h"

MyQtGeoShapeBase::MyQtGeoShapeBase(qint32 geoShapeObjectID, QString name, QObject *parent) : QObject(parent)
{
    setGeoShapObjectID(geoShapeObjectID);
    setName(name);
}

Enum_MyQtGeoShapeType MyQtGeoShapeBase::getGeoShapeType() const
{
    return EV_MyQtGeoShapeType_Base;
}

qint32 MyQtGeoShapeBase::getGeoShapObjectID() const
{
    return geoShapObjectID;
}

void MyQtGeoShapeBase::setGeoShapObjectID(const qint32 &value)
{
    geoShapObjectID = value;
}

QString MyQtGeoShapeBase::getName() const
{
    return name;
}

void MyQtGeoShapeBase::setName(const QString &value)
{
    name = value;
}
