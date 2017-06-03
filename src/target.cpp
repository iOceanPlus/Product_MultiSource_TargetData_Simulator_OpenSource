#include <QDateTime>
#include "target.h"
#include "macro.h"
#include "PBCoderDecoder.h"
#include "ParallelWorld.h"
#include <QGeoCoordinate>
#include <QDebug>
#include <QtMath>

Target::Target(PBTargetPosition pbTargetPos, ParallelWorld *paramParallelWorld,qint32 secondsWindowSizeOfTargetPos)
{
    this->pbTargetPosFusedLast.CopyFrom(pbTargetPos);
    this->parallelWorld=paramParallelWorld;

}

Target::~Target()
{

}


//获得deltaSeconds后的船位置等
PBAISDynamic Target::getReckonedAISDynamic(const PBAISDynamic &pbAISDynamic, const qint32 &deltaSecondsLater, double &stdDevDistance)
{
    PBAISDynamic aisDynamicCurrent=pbAISDynamic;
    PBAISDynamic aisDynamicReckoned=pbAISDynamic;

    // as an uint32, aisDynamicCurrent.sogknotsx10()>=0 is always true
    {
        double distance=aisDynamicCurrent.sogknotsx10()/10*NM_In_Meter/3600.0*deltaSecondsLater;
        // as an uint32, aisDynamicCurrent.cogdegreex10()>=0 is always true
        if(aisDynamicCurrent.cogdegreex10()<=3599) //valid cog
        {
            QGeoCoordinate geo(aisDynamicCurrent.intlatitudex60w()/AISPosDivider,aisDynamicCurrent.intlongitudex60w()/AISPosDivider);
            QGeoCoordinate geoReckoned=geo.atDistanceAndAzimuth(distance,aisDynamicCurrent.cogdegreex10()/10.0,0);
            aisDynamicReckoned.set_intlatitudex60w(geoReckoned.latitude()*AISPosDivider);
            aisDynamicReckoned.set_intlongitudex60w(geoReckoned.longitude()*AISPosDivider);
            stdDevDistance=distance*0.1; //0.1 is empirical set
        }
        else //no valid cog
        {
            stdDevDistance=distance;
        }
    }

    aisDynamicReckoned.set_utctimestamp(aisDynamicCurrent.utctimestamp()+deltaSecondsLater);
    return aisDynamicReckoned;
}


const PBTargetPosition& Target::getConstPbTargetPosFusedLast() const
{
    return pbTargetPosFusedLast;
}

PBTargetPosition & Target::getPbTargetPosFusedLastRef()
{
    return pbTargetPosFusedLast;
}


quint64 Target::getTargetIDOrigAggregatedWithIDType(const quint8 &targetID_Type, const quint32 &targetIDOrig)
{
    quint64 targetIDToReturn=targetID_Type;
    targetIDToReturn=(targetIDToReturn<<56)+targetIDOrig;
    return targetIDToReturn;
}
