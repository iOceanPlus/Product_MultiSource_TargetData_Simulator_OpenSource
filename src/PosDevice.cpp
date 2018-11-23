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
        PBTargetPosition pbTargetPos=targetInstalled->updateTargetAndGetPbTargetPosCurrent(currentDTAsMSecsSinceEpoch);
        pbTargetPos.set_enum_targetinfotype(infoType);
        targetInstalled->set_enum_targetidorigAndIDType_AccordingToInfoType(pbTargetPos);
#ifdef DEBUG_TARGETTYPE_ANDNAME
        qDebug()<<pbTargetPos.aisdynamic().mmsi()<<pbTargetPos.beidouid()<<pbTargetPos.haijianid()<<pbTargetPos.argosandmarinesatid()
               <<":"<<pbTargetPos.aisstatic().shiptype_ais()<<pbTargetPos.aggregatedaisshiptype()<<
                  QString::fromStdString(pbTargetPos.aisstatic().shipname());
#endif
        targetInstalled->clearInvalidFieldsInAnOriginalTargetPos(pbTargetPos);

        addDevToDynamicData(pbTargetPos);
        return pbTargetPos;
    }
}

bool  PosDevice::addDevToDynamicData(PBTargetPosition &pbTargetPos)
{
    if(targetInstalled->getDeviceInfo(infoType).positioningDevInMeters>0)
    {
        std::normal_distribution<double> normalDistDistance(0.0, targetInstalled->getDeviceInfo(infoType).positioningDevInMeters);
        double distanceInMeters=normalDistDistance( *(targetInstalled->getRandEngineOfThisWorld()) );
        double azimuth=qrand()%3600/10.0;
        QGeoCoordinate geo(pbTargetPos.aisdynamic().intlatitudex60w()/AISPosDivider,pbTargetPos.aisdynamic().intlongitudex60w()/AISPosDivider);
        QGeoCoordinate geoReckoned=geo.atDistanceAndAzimuth(qAbs(distanceInMeters),azimuth,0);
        pbTargetPos.mutable_aisdynamic()->set_intlongitudex60w(qRound(geoReckoned.longitude()*AISPosDivider));
        pbTargetPos.mutable_aisdynamic()->set_intlatitudex60w(qRound(geoReckoned.latitude()*AISPosDivider));
    }

    if(targetInstalled->getDeviceInfo(infoType).SOGDevInKnots>0)
    {
        std::normal_distribution<double> normalDistSOG(0.0, targetInstalled->getDeviceInfo(infoType).SOGDevInKnots);
        double sogX10=pbTargetPos.aisdynamic().sogknotsx10()+10*normalDistSOG(* (targetInstalled->getRandEngineOfThisWorld()));
        sogX10=sogX10<0?0:sogX10;
        pbTargetPos.mutable_aisdynamic()->set_sogknotsx10(qRound(sogX10) );
    }

    if(targetInstalled->getDeviceInfo(infoType).COGDevInDegrees>0)
    {
        std::normal_distribution<double> normalDistCOG(0.0, targetInstalled->getDeviceInfo(infoType).COGDevInDegrees);
        double cogX10=pbTargetPos.aisdynamic().cogdegreex10()+10*normalDistCOG(* (targetInstalled->getRandEngineOfThisWorld()));
        cogX10=cogX10<0?cogX10+3600:cogX10;
        pbTargetPos.mutable_aisdynamic()->set_cogdegreex10(qRound(cogX10)%3600);
    }

    return true;
}
