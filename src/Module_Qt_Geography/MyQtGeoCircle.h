#ifndef MYQTGEOCIRCLE_H
#define MYQTGEOCIRCLE_H

#include <QObject>
#include <QGeoCircle>
#include <MyQtGeoShapeBase.h>

class MyQtGeoCircle : public MyQtGeoShapeBase
{
public:
    explicit MyQtGeoCircle(const QGeoCoordinate &center, qreal radiusInMeters,bool *ok,
                            qint32 geoShapeObjectID, QString name, QObject *parent = 0);
    virtual ~MyQtGeoCircle() {}
    virtual bool containsPoint(QGeoCoordinate geoCoordinate, Qt::FillRule fillRule=Qt::OddEvenFill) ;
    virtual Enum_MyQtGeoShapeType getGeoShapeType() const;

    QGeoCircle getGeoCircle() const;
    void setGeoCircle(const QGeoCircle &value);

protected:
    QGeoCircle geoCircle;
};

#endif // MYQTGEOCIRCLE_H
