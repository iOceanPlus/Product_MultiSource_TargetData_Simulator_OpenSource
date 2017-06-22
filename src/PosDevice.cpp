#include <QDateTime>
#include <QGeoCoordinate>
#include <QDebug>
#include "PosDevice.h"
#include "target.h"
#include "macro.h"

PosDevice::PosDevice(const QDateTime &lastDeviceSampleTime, Target * const target, const PB_Enum_TargetInfo_Type &infoType)
{
    this->targetInstalled=target;
    this->lastSampleTime= lastDeviceSampleTime;
    this->infoType=infoType;
}

PBTargetPosition PosDevice::measurePBTargetPosAndUpdateTarget(bool &isMeasureSuccessful)
{
    QDateTime currentTime=QDateTime::currentDateTime();
    qint64 msecondsElapsed=lastSampleTime.msecsTo(currentTime);
    if(msecondsElapsed<targetInstalled->getDeviceInfo(infoType).sampleMilliSeconds)
    {
        isMeasureSuccessful=false;
        return PBTargetPosition::default_instance();
    }
    else
    {
        isMeasureSuccessful=true;
        lastSampleTime=currentTime;
        PBTargetPosition pbTargetPos=targetInstalled->updateAndGetPbTargetPosCurrent();
        pbTargetPos.set_enum_targetinfotype(infoType);
        targetInstalled->set_enum_targetidorigAndIDType_AccordingToInfoType(pbTargetPos);
        targetInstalled->clearInvalidFieldsInAnOriginalTargetPos(pbTargetPos);

        addDevToPos(pbTargetPos);
        return pbTargetPos;
    }
}

bool  PosDevice::addDevToPos(PBTargetPosition &pbTargetPos)
{
    double distance=targetInstalled->getDeviceInfo(infoType).positioningDevInMeters*(qrand()%10000/10000.0);
    double azimuth=qrand()%3600/10.0;

    QGeoCoordinate geo(pbTargetPos.aisdynamic().intlatitudex60w()/AISPosDivider,pbTargetPos.aisdynamic().intlongitudex60w()/AISPosDivider);
    QGeoCoordinate geoReckoned=geo.atDistanceAndAzimuth(distance,azimuth,0);
    pbTargetPos.mutable_aisdynamic()->set_intlongitudex60w(qRound(geoReckoned.longitude()*AISPosDivider));
    pbTargetPos.mutable_aisdynamic()->set_intlatitudex60w(qRound(geoReckoned.latitude()*AISPosDivider));
    return true;
}
