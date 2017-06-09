#include <QDateTime>
#include "target.h"
#include "macro.h"
#include "PBCoderDecoder.h"
#include "ParallelWorld.h"
#include <QGeoCoordinate>
#include <QDebug>
#include <QtMath>

Target::Target(const PBTargetPosition &pbTargetPos,
               ParallelWorld *paramParallelWorld, const QDateTime &posDateTime)
{
    this->hashTargetInfoTypePosDevice=hashTargetInfoTypePosDevice;
    this->parallelWorld=paramParallelWorld;

    this->pbTargetPosOrig.CopyFrom(pbTargetPos);
    this->posOrigDateTime=posDateTime;

    this->pbTargetPosCurrent.CopyFrom(pbTargetPos);
    this->posCurrentDateTime=posDateTime;
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
            else
                newCOGX10%=3600;

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

bool Target::addPosDevice(PB_Enum_TargetInfo_Type infoType, PosDevice* posDev)
{
    hashTargetInfoTypePosDevice.insert(infoType,posDev);
    return true;
}

bool Target::installPosDevices()
{







}

/***************data cleaning*******************/
void  Target::setOriginalTargetIDsOfTargetPos(PBTargetPosition &pbTargetPosToSet)
{
    if(pbTargetPosToSet.enum_targetidorig_type()!=EV_TargetIDType_MMSI)
    {
        pbTargetPosToSet.mutable_aisdynamic()->set_mmsi(-1);
        pbTargetPosToSet.mutable_aisstatic()->set_mmsi(-1);
        pbTargetPosToSet.mutable_aisvoyage()->set_mmsi(-1);
    }

    if(pbTargetPosToSet.enum_targetidorig_type()==EV_TargetIDType_BeidouID)
    {
        pbTargetPosToSet.set_beidouid(pbTargetPosToSet.targetidorig());
        pbTargetPosToSet.set_haijianid(-1);
        pbTargetPosToSet.set_argosandmarinesatid(-1);
    }

    if(pbTargetPosToSet.enum_targetidorig_type()==EV_TargetIDType_HaijianID)
    {
        pbTargetPosToSet.set_beidouid(-1);
        pbTargetPosToSet.set_haijianid(pbTargetPosToSet.targetidorig());
        pbTargetPosToSet.set_argosandmarinesatid(-1);
    }

    if(pbTargetPosToSet.enum_targetidorig_type()==EV_TargetIDType_ArgosAndMarineSatID)
    {
        pbTargetPosToSet.set_beidouid(-1);
        pbTargetPosToSet.set_haijianid(-1);
        pbTargetPosToSet.set_argosandmarinesatid(pbTargetPosToSet.targetidorig());
    }
}

void Target::clearInvalidFieldsInAnOriginalTargetPos(PBTargetPosition &pbTargetPos)
{
    if(pbTargetPos.enum_targetidorig_type()!=EV_TargetIDType_MMSI)
    {
        pbTargetPos.mutable_aisdynamic()->set_mmsi(-1);
        pbTargetPos.mutable_aisstatic()->set_mmsi(-1);
        pbTargetPos.mutable_aisvoyage()->set_mmsi(-1);
    }
    if(pbTargetPos.enum_targetidorig_type()!=EV_TargetIDType_BeidouID)
        pbTargetPos.set_beidouid(-1);
    if(pbTargetPos.enum_targetidorig_type()!=EV_TargetIDType_HaijianID)
        pbTargetPos.set_haijianid(-1);
    if(pbTargetPos.enum_targetidorig_type()!=EV_TargetIDType_ArgosAndMarineSatID)
        pbTargetPos.set_argosandmarinesatid(-1);
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
