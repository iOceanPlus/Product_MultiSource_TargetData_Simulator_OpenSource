#include "MyQtGeoPolygon.h"
#include <QDebug>
#include <QLineF>

MyQtGeoPolygon::MyQtGeoPolygon(QVector<QPointF> paramGeoPointsInDegreesBeforeTranslate, bool *ok, qint32 geoShapeObjectID,
                               QString name, QObject *parent) : MyQtGeoShapeBase(geoShapeObjectID,name,parent)
{
    this->geoPointsInDegreesBeforeTranslate=paramGeoPointsInDegreesBeforeTranslate;
    this->geoPointsInDegreeAfterTranslate=paramGeoPointsInDegreesBeforeTranslate;
    is180LongitudeCrossed=false;
    QPolygonF polygonFTest(paramGeoPointsInDegreesBeforeTranslate);
    if(  !polygonFTest.isClosed())
    {
        qDebug()<<"Error: polygon is not closed! Make sure that starting point and ending point are the same!";
       *ok=false;
        return;
    }
    if(polygonFTest.size()<3)
    {
        qDebug()<<"Error: There are less than 3 points!";
        *ok=false;
        return;
    }

    if(setIs180LongitudeCrossedAndCheckPointsValidity()&&translatePointsWhenNeeded())
        *ok=true;
    else
    {
        *ok=false;
        return;
    }

    polygonFTranslated=geoPointsInDegreeAfterTranslate;
}

bool MyQtGeoPolygon:: translatePointsWhenNeeded()
{
    if(is180LongitudeCrossed)
    {
        QMutableVectorIterator <QPointF> i(geoPointsInDegreeAfterTranslate);
        QPointF  pointF;
        while (i.hasNext())
        {
            pointF=i.next();
            if(pointF.x()<0)
            {
                pointF.setX(360+pointF.x());
                i.setValue(pointF);
            }
        }
    }

    return true;
}

bool MyQtGeoPolygon::setIs180LongitudeCrossedAndCheckPointsValidity()
{
    //QPointF southPole(180,-90),northPole(180,90);
  //  QLineF lineF180DegreeLongitude(southPole,northPole); //longitude of the degree 180 degrees

    QVectorIterator <QPointF> i(geoPointsInDegreesBeforeTranslate);
    QPointF lastPointF;
    bool isLastPointValid=false;
    while (i.hasNext())
    {
        if(!isLastPointValid)
        {
            lastPointF=i.next();
            if(!checkPointFValidity(lastPointF))
            {
                return false;
             }
            isLastPointValid=true;
        }
        else
        {
            QPointF currentPoint=i.next();
            if(!checkPointFValidity(currentPoint))
            {
                return false;
            }
            if(currentPoint.x()*lastPointF.x()<0) //one positive longitude, one negative longitude
            {
                QLineF lineF(lastPointF,currentPoint);
                if(lineF.dx()>180) //180 degree crossed
                {
                    is180LongitudeCrossed=true;
                }
            }
            lastPointF=currentPoint;
        }
    }
    return true;
}

bool MyQtGeoPolygon::containsPoint( QGeoCoordinate geoCoordinate, Qt::FillRule fillRule)
{
    if(!geoCoordinate.isValid())
        qDebug()<<"Error: invalid geoCooridinate:"<<geoCoordinate;
    QPointF pointF(geoCoordinate.longitude(),geoCoordinate.latitude());

    if(pointF.x()<0&&is180LongitudeCrossed)
        pointF.setX(pointF.x()+360);

    return polygonFTranslated.containsPoint(pointF,fillRule);
}

Enum_MyQtGeoShapeType MyQtGeoPolygon::getGeoShapeType() const
{
    return EV_MyQtGeoShapeType_Polygon;
}


bool MyQtGeoPolygon::checkPointFValidity(QPointF &pointF)
{
    if(pointF.x()<-180||pointF.x()>180||pointF.y()>90||pointF.y()<-90)
    {
        qDebug()<<"Invalid points:"<<pointF;
        return false;
    }
    return true;
}

QVector<QPointF> MyQtGeoPolygon::getGeoPointsInDegreeAfterTranslate() const
{
    return geoPointsInDegreeAfterTranslate;
}

QVector<QPointF> MyQtGeoPolygon::getGeoPointsInDegreesBeforeTranslate() const
{
    return geoPointsInDegreesBeforeTranslate;
}

QPolygonF MyQtGeoPolygon::getPolygonFTranslated() const
{
    return polygonFTranslated;
}

