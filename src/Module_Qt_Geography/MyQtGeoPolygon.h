#ifndef MYQTGEOPOLYGON_H
#define MYQTGEOPOLYGON_H

#include <QObject>
#include <QPolygonF>
#include "MyQtGeoShapeBase.h"

class MyQtGeoPolygon : public MyQtGeoShapeBase
{
    Q_OBJECT
public:
    // use x to represent longitude, y to represent latitude.
    // Note the difference from QGeoCoordinate: first parameter is latitude, second parameter is longitude.
    explicit MyQtGeoPolygon(QVector<QPointF> paramGeoPointsInDegreesBeforeTranslate, bool *ok,
                            qint32 geoShapeObjectID, QString name,QObject *parent = 0);
    virtual ~MyQtGeoPolygon() {}

    virtual bool containsPoint(QGeoCoordinate geoCoordinate, Qt::FillRule fillRule=Qt::OddEvenFill) ;
    virtual Enum_MyQtGeoShapeType getGeoShapeType() const;

    QPolygonF getPolygonFTranslated() const;
    QVector<QPointF> getGeoPointsInDegreesBeforeTranslate() const;
    QVector<QPointF> getGeoPointsInDegreeAfterTranslate() const;
signals:

public slots:

protected:
    bool setIs180LongitudeCrossedAndCheckPointsValidity();
    bool translatePointsWhenNeeded();
    bool checkPointFValidity(QPointF &pointF);

    QPolygonF polygonFTranslated; //Polygon after negative longitude values adding 360 (when 180 longitude crossed)
    QVector<QPointF> geoPointsInDegreesBeforeTranslate,geoPointsInDegreeAfterTranslate;

    bool is180LongitudeCrossed; //if crossed, add 360 degrees to negative longitude values
};

#endif // MYQTGEOPOLYGON_H
