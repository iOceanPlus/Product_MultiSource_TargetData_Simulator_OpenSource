#include <QDateTime>
#include <QGeoCoordinate>
#include "posdevice.h"
#include "target.h"
#include "macro.h"

PosDevice::PosDevice(const PB_Enum_TargetInfo_Type   &targetInfoType, Target * const targetInstalled,
                     qint64 sampleMilliSeconds, QDateTime lastDeviceSampleTime, const double &positioningDevInMeters)
{
    this->targetInfoType=targetInfoType;
    this->targetInstalled=targetInstalled;
    this->sampleMilliSeconds=sampleMilliSeconds;
    this->positioningDevInMeters=positioningDevInMeters;
    this->lastSampleTime= lastDeviceSampleTime;
}

PBTargetPosition PosDevice::measurePBTargetPosAndUpdateTarget(bool &isMeasureSuccessful)
{
    QDateTime currentTime=QDateTime::currentDateTime();
    qint64 msecondsElapsed=lastSampleTime.msecsTo(currentTime);
    if(msecondsElapsed<sampleMilliSeconds)
    {
        isMeasureSuccessful=false;
        return PBTargetPosition::default_instance();
    }
    else
    {
        isMeasureSuccessful=true;
        lastSampleTime=currentTime;
        PBTargetPosition pbTargetPos=targetInstalled->updateAndGetPbTargetPosCurrent();
        addDevToPos(pbTargetPos);
        return pbTargetPos;
    }
}

bool  PosDevice::addDevToPos(PBTargetPosition &pbTargetPos)
{
    double distance=positioningDevInMeters*(qrand()%10000/10000.0);
    double azimuth=qrand()%3600/3600.0;

    QGeoCoordinate geo(pbTargetPos.aisdynamic().intlatitudex60w()/AISPosDivider,pbTargetPos.aisdynamic().intlongitudex60w()/AISPosDivider);
    QGeoCoordinate geoReckoned=geo.atDistanceAndAzimuth(distance,azimuth,0);
    pbTargetPos.mutable_aisdynamic()->set_intlongitudex60w(qRound(geoReckoned.longitude()*AISPosDivider));
    pbTargetPos.mutable_aisdynamic()->set_intlatitudex60w(qRound(geoReckoned.latitude()*AISPosDivider));
    return true;
}
