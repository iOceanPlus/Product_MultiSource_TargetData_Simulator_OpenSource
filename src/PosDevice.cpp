#include <QGeoCoordinate>
#include <QDebug>
#include "PosDevice.h"
#include "target.h"
#include "macro.h"

PosDevice::PosDevice(const qint64 &lastDeviceSampleTimeAsMSecsSinceEpoch, Target * const target, const PB_Enum_TargetInfo_Type &infoType)
{
    this->targetInstalled=target;
    this->msecsSinceEpochLastSampleTime= lastDeviceSampleTimeAsMSecsSinceEpoch;
    this->infoType=infoType;
}

PBTargetPosition PosDevice::measurePBTargetPosAndUpdateTarget(bool &isMeasureSuccessful, const qint64 &currentDTAsMSecsSinceEpoch)
{
    qint64 msecondsElapsed=currentDTAsMSecsSinceEpoch-msecsSinceEpochLastSampleTime;
    if(msecondsElapsed<targetInstalled->getDeviceInfo(infoType).sampleMilliSeconds)
    {
        isMeasureSuccessful=false;
        return PBTargetPosition::default_instance();
    }
    else
    {
        isMeasureSuccessful=true;
        msecsSinceEpochLastSampleTime=currentDTAsMSecsSinceEpoch;
        PBTargetPosition pbTargetPos=targetInstalled->updateAndGetPbTargetPosCurrent(currentDTAsMSecsSinceEpoch);
        pbTargetPos.set_enum_targetinfotype(infoType);
        targetInstalled->set_enum_targetidorigAndIDType_AccordingToInfoType(pbTargetPos);
#ifdef DEBUG_TARGETTYPE_ANDNAME
        qDebug()<<pbTargetPos.aisdynamic().mmsi()<<pbTargetPos.beidouid()<<pbTargetPos.haijianid()<<pbTargetPos.argosandmarinesatid()
               <<":"<<pbTargetPos.aisstatic().shiptype_ais()<<pbTargetPos.aggregatedaisshiptype()<<
                  QString::fromStdString(pbTargetPos.aisstatic().shipname());
#endif
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
