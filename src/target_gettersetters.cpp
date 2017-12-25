#include "target.h"
#include "macro.h"
#include "PBCoderDecoder.h"
#include "ThreadedWorld.h"
#include <QGeoCoordinate>
#include <QDebug>
#include <QtMath>

bool Target::addPosDevice(PB_Enum_TargetInfo_Type infoType, PosDevice* posDev)
{
    hashTargetInfoTypePosDevice.insert(infoType,posDev);
    return true;
}

PBTargetPosition Target::getPBTargetPosCurrent() const
{
    PBTargetPosition pbTargetPosToReturn=pbTargetPosInitial;
    pbTargetPosToReturn.mutable_aisdynamic()->set_cogdegreex10( qRound(kinematicCurrent.cogInDegreesHighPreci*10));
    pbTargetPosToReturn.mutable_aisdynamic()->set_headingdegree(qRound(kinematicCurrent.cogInDegreesHighPreci));
    pbTargetPosToReturn.mutable_aisdynamic()->set_utctimestamp(qRound(kinematicCurrent.dateTimeMSecs/1000.0));
    pbTargetPosToReturn.mutable_aisdynamic()->set_sogknotsx10(
                qRound(kinematicCurrent.speedMetersPerSecondCurrentHighPreci/KNOT_IN_METER_PER_SECOND*10));
    pbTargetPosToReturn.mutable_aisdynamic()->set_intlatitudex60w(qRound(kinematicCurrent.geoHighPreci.latitude()*AISPosDivider));
    pbTargetPosToReturn.mutable_aisdynamic()->set_intlongitudex60w(qRound(kinematicCurrent.geoHighPreci.longitude()*AISPosDivider));
    return pbTargetPosToReturn;
}

/***************data cleaning*******************/
void  Target::setOriginalTargetIDsOfTargetPos(PBTargetPosition &pbTargetPosToSet)
{
    if(pbTargetPosToSet.enum_targetidorig_type()!=EV_TargetIDType_MMSI)
    {
        pbTargetPosToSet.mutable_aisdynamic()->set_mmsi(0);
        pbTargetPosToSet.mutable_aisstatic()->set_mmsi(0);
        pbTargetPosToSet.mutable_aisvoyage()->set_mmsi(0);
    }

    if(pbTargetPosToSet.enum_targetidorig_type()==EV_TargetIDType_BeidouID)
    {
        pbTargetPosToSet.set_beidouid(pbTargetPosToSet.targetidorig());
        pbTargetPosToSet.set_haijianid(0);
        pbTargetPosToSet.set_argosandmarinesatid(0);
    }

    if(pbTargetPosToSet.enum_targetidorig_type()==EV_TargetIDType_HaijianID)
    {
        pbTargetPosToSet.set_beidouid(0);
        pbTargetPosToSet.set_haijianid(pbTargetPosToSet.targetidorig());
        pbTargetPosToSet.set_argosandmarinesatid(0);
    }

    if(pbTargetPosToSet.enum_targetidorig_type()==EV_TargetIDType_ArgosAndMarineSatID)
    {
        pbTargetPosToSet.set_beidouid(0);
        pbTargetPosToSet.set_haijianid(0);
        pbTargetPosToSet.set_argosandmarinesatid(pbTargetPosToSet.targetidorig());
    }
}

void Target::clearInvalidFieldsInAnOriginalTargetPos(PBTargetPosition &pbTargetPos)
{
    if(pbTargetPos.enum_targetidorig_type()!=EV_TargetIDType_MMSI)
    {
        pbTargetPos.mutable_aisdynamic()->set_mmsi(0);
        pbTargetPos.mutable_aisstatic()->set_mmsi(0);
        pbTargetPos.mutable_aisvoyage()->set_mmsi(0);
    }
    if(pbTargetPos.enum_targetidorig_type()!=EV_TargetIDType_BeidouID)
        pbTargetPos.set_beidouid(0);
    if(pbTargetPos.enum_targetidorig_type()!=EV_TargetIDType_HaijianID)
        pbTargetPos.set_haijianid(0);
    if(pbTargetPos.enum_targetidorig_type()!=EV_TargetIDType_ArgosAndMarineSatID)
        pbTargetPos.set_argosandmarinesatid(0);
}

void  Target::set_enum_targetidorigAndIDType_AccordingToInfoType(PBTargetPosition &pbTargetPosToSet)
{
    PB_Enum_TargetInfo_Type infoType= pbTargetPosToSet.enum_targetinfotype();
    switch (infoType) {
    case EV_TargetInfoType_NA:
        pbTargetPosToSet.set_enum_targetidorig_type(EV_TargetIDType_NA);
        pbTargetPosToSet.set_targetidorig(-1);
        break;
    case EV_TargetInfoType_ArgosAndMaritimeSatellite:
        pbTargetPosToSet.set_enum_targetidorig_type(EV_TargetIDType_ArgosAndMarineSatID);
        pbTargetPosToSet.set_targetidorig(pbTargetPosToSet.argosandmarinesatid());
        break;
    case EV_TargetInfoType_Beidou:
        pbTargetPosToSet.set_enum_targetidorig_type(EV_TargetIDType_BeidouID);
        pbTargetPosToSet.set_targetidorig(pbTargetPosToSet.beidouid());
        break;
    case EV_TargetInfoType_AISStatic:
    case EV_TargetInfoType_AISDynamic:
    case EV_TargetInfoType_LRIT:
        pbTargetPosToSet.set_enum_targetidorig_type(EV_TargetIDType_MMSI);
        pbTargetPosToSet.set_targetidorig(pbTargetPosToSet.aisdynamic().mmsi());
        break;
    case EV_TargetInfoType_Haijian:
        pbTargetPosToSet.set_enum_targetidorig_type(EV_TargetIDType_HaijianID);
        pbTargetPosToSet.set_targetidorig(pbTargetPosToSet.haijianid());
        break;
    default:
        pbTargetPosToSet.set_enum_targetidorig_type(EV_TargetIDType_Others);
        pbTargetPosToSet.set_targetidorig(-1);
        break;
    }

    clearInvalidFieldsInAnOriginalTargetPos(pbTargetPosToSet);
}

QHash<PB_Enum_TargetInfo_Type, PosDevice*> Target::getHashTargetInfoTypePosDevice() const
{
    return hashTargetInfoTypePosDevice;
}
const QGeoCoordinate Target::getConstGeoPosHighPreciReckoned(const QGeoCoordinate &geoOrig, const double metersTravelled,
                                                             const double &degreeAziumth, bool &isOnLand) const
{
    QGeoCoordinate geoReckoned=geoOrig.atDistanceAndAzimuth(metersTravelled,degreeAziumth,0);
    isOnLand=!world->isInWaterAndBoudingArea(geoReckoned.longitude(),geoReckoned.latitude());
    return geoReckoned;
}

const QGeoCoordinate Target::getConstGeoPosHighPreciReckoned(const QGeoCoordinate &geoOrig, const  qint64 &dtOrigMSecs,
                        const double &avgSpeedInMetersPerSecond, const double &degreeAziumth,  const qint64 &dtToReckonMSecs, bool &isOnLand) const
{
    if(avgSpeedInMetersPerSecond<=0)
    {
        isOnLand=!world->isInWaterAndBoudingArea(geoOrig.longitude(),geoOrig.latitude());
        return geoOrig;
    }

    qint64 miliSecondsElapsed=dtToReckonMSecs-dtOrigMSecs;
    double distance=avgSpeedInMetersPerSecond*miliSecondsElapsed/1000.0;
    return getConstGeoPosHighPreciReckoned(geoOrig,distance,degreeAziumth,isOnLand);
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
    return world->getMapInfoTypePosDeviceInfo().value(infoType);
}

quint64 Target::getTargetIDOrigAggregatedWithIDType(const quint8 &targetID_Type, const quint32 &targetIDOrig)
{
    quint64 targetIDToReturn=targetID_Type;
    targetIDToReturn=(targetIDToReturn<<56)+targetIDOrig;
    return targetIDToReturn;
}

std::default_random_engine *Target::getRandEngineOfThisWorld() const
{
    return world->getRandomEngine();
}

