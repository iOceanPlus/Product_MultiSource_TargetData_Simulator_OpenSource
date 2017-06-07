#include <QDateTime>
#include "target.h"
#include "macro.h"
#include "PBCoderDecoder.h"
#include "ParallelWorld.h"
#include <QGeoCoordinate>
#include <QDebug>
#include <QtMath>

Target::Target(const QHash<PB_Enum_TargetInfo_Type, PosDevice *> &hashTargetInfoTypePosDevice, const PBTargetPosition &pbTargetPos,
               ParallelWorld *paramParallelWorld, const QDateTime &posDateTime)
{
    this->hashTargetInfoTypePosDevice=hashTargetInfoTypePosDevice;
    this->pbTargetPosOrig.CopyFrom(pbTargetPos);
    this->parallelWorld=paramParallelWorld;
    this->posOrigDateTime=posDateTime;
}

Target::~Target()
{

}


void Target::updateTargetPosCurrentAndOrigIfMeetLand()
{
    bool isOnLand;
    QDateTime currentDateTime=QDateTime::currentDateTime();
    PBTargetPosition pbTargetPosReckoned=getReckonedPbTargetPos(currentDateTime,isOnLand);
    if(pbTargetPosReckoned.aisdynamic().sogknotsx10()>0&& isOnLand)
    {
        pbTargetPosOrig.CopyFrom(pbTargetPosCurrent);
        posOrigDateTime=posCurrentDateTime;
        bool turnRight=qrand()%2==0;
        bool newCOGGot=false;
        for(int i=1;i<=7;i++)
        {
            qint32 newCOGX10=pbTargetPosOrig.aisdynamic().cogdegreex10()+turnRight*DegreesX10_ToTurn_WhenMeetLand;
            if(newCOGX10<0)
                newCOGX10+=3600;

            pbTargetPosOrig.mutable_aisdynamic()->set_cogdegreex10(newCOGX10);
            bool isNewPosOnLand;
            pbTargetPosReckoned=getReckonedPbTargetPos(currentDateTime,isNewPosOnLand);
            if(isNewPosOnLand)
                continue;
            else
            {
                newCOGGot=true;
                break;
            }
        }
        if(newCOGGot)
        {
            pbTargetPosOrig.CopyFrom(pbTargetPosReckoned);
            pbTargetPosCurrent.CopyFrom(pbTargetPosReckoned);
            posOrigDateTime=currentDateTime;
            posCurrentDateTime=currentDateTime;
        }
        else
        {
            pbTargetPosOrig.mutable_aisdynamic()->set_sogknotsx10(0);
            pbTargetPosCurrent.mutable_aisdynamic()->set_sogknotsx10(0);
        }
    }
    else if(!isOnLand)
    {
        pbTargetPosCurrent.CopyFrom(pbTargetPosReckoned);
        posCurrentDateTime=currentDateTime;
    }
}

QHash<PB_Enum_TargetInfo_Type, PosDevice*> Target::getHashTargetInfoTypePosDevice() const
{
    return hashTargetInfoTypePosDevice;
}

PBTargetPosition Target::updateAndGetPbTargetPosCurrent()
{
    updateTargetPosCurrentAndOrigIfMeetLand();
    return pbTargetPosCurrent;
}

PBTargetPosition Target::getReckonedPbTargetPos(const QDateTime &dtToReckon, bool &isOnLand) const
{
    PBTargetPosition pbTargetPosReckoned;
    pbTargetPosReckoned.CopyFrom(pbTargetPosOrig);
    if(pbTargetPosOrig.aisdynamic().sogknotsx10()==0)
    {
        pbTargetPosReckoned.mutable_aisdynamic()->set_utctimestamp(dtToReckon.toTime_t());
        return pbTargetPosReckoned;
    }

    PBAISDynamic aisDynamicCurrent=pbTargetPosOrig.aisdynamic();
    qint64 miliSecondsElapsed=posOrigDateTime.msecsTo(dtToReckon);
    double distance=aisDynamicCurrent.sogknotsx10()/10.0*NM_In_Meter/3600.0*miliSecondsElapsed/1000.0;
    QGeoCoordinate geo(aisDynamicCurrent.intlatitudex60w()/AISPosDivider,aisDynamicCurrent.intlongitudex60w()/AISPosDivider);
    QGeoCoordinate geoReckoned=geo.atDistanceAndAzimuth(distance,aisDynamicCurrent.cogdegreex10()/10.0,0);
    pbTargetPosReckoned.mutable_aisdynamic()->set_intlongitudex60w(geoReckoned.longitude()*AISPosDivider);
    pbTargetPosReckoned.mutable_aisdynamic()->set_intlatitudex60w(geoReckoned.latitude()*AISPosDivider);
    pbTargetPosReckoned.mutable_aisdynamic()->set_utctimestamp(dtToReckon.toTime_t());

    if(parallelWorld->isInWater(geoReckoned.longitude(),geoReckoned.latitude()))
    {
        isOnLand=false;
    }
    else
        isOnLand=true;

    return pbTargetPosReckoned;
}

PosDevice* Target::getDevice(const PB_Enum_TargetInfo_Type &infoType)
{
    if(!hashTargetInfoTypePosDevice.contains(infoType))
        return NULL;
    else
        return hashTargetInfoTypePosDevice.value(infoType);
}

const Struct_PosDeviceInfo Target::getDeviceInfo(const PB_Enum_TargetInfo_Type &infoType) const
{
    return parallelWorld->getMapInfoTypePosDeviceInfo().value(infoType);
}

quint64 Target::getTargetIDOrigAggregatedWithIDType(const quint8 &targetID_Type, const quint32 &targetIDOrig)
{
    quint64 targetIDToReturn=targetID_Type;
    targetIDToReturn=(targetIDToReturn<<56)+targetIDOrig;
    return targetIDToReturn;
}
