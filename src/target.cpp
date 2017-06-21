#include <QDateTime>
#include "target.h"
#include "macro.h"
#include "PBCoderDecoder.h"
#include "World.h"
#include <QGeoCoordinate>
#include <QDebug>
#include <QtMath>

Target::Target(const PBTargetPosition &pbTargetPos,
               World *paramWorld, const QDateTime &posDateTime)
{
    this->hashTargetInfoTypePosDevice=hashTargetInfoTypePosDevice;
    this->world=paramWorld;

    this->pbTargetPosOrig.CopyFrom(pbTargetPos);
    this->posOrigDateTime=posDateTime;

    this->pbTargetPosCurrent.CopyFrom(pbTargetPos);
    this->pbTargetPosBeforeCurrent.CopyFrom(pbTargetPos);
    this->posCurrentDateTime=posDateTime;
}

Target::~Target()
{
    QHashIterator <PB_Enum_TargetInfo_Type, PosDevice*> iHashTargetInfoTypePosDevice(hashTargetInfoTypePosDevice);
    while(iHashTargetInfoTypePosDevice.hasNext())
    {
        iHashTargetInfoTypePosDevice.next();
        delete iHashTargetInfoTypePosDevice.value();
    }
}


void Target::updateTargetPosCurrentAndOrigIfMeetLand()
{
    bool isOnLand;
    QDateTime currentDateTime=QDateTime::currentDateTime();
    if(posCurrentDateTime.msecsTo(currentDateTime)<MIN_Sample_MSEC)
    {
        return;
    }

    if(pbTargetPosCurrent.aisdynamic().sogknotsx10()<=0)
    {
         posCurrentDateTime=currentDateTime;
         pbTargetPosCurrent.mutable_aisdynamic()->set_utctimestamp(currentDateTime.toTime_t());
         return;
    }

    reckonPbTargetPosCurrentAndCalibrateCOG(currentDateTime,isOnLand);
    if(pbTargetPosCurrent.aisdynamic().sogknotsx10()>0&& isOnLand)
    {
        pbTargetPosOrig.CopyFrom(pbTargetPosBeforeCurrent);
        pbTargetPosCurrent.CopyFrom(pbTargetPosBeforeCurrent);
        posOrigDateTime=QDateTime::fromTime_t(pbTargetPosOrig.aisdynamic().utctimestamp());
        posCurrentDateTime=posOrigDateTime;
        bool turnRight=qrand()%2==0;
        bool newCOGGot=false;
        for(int i=1;i<=7;i++)
        {
            qint32 newCOGX10=pbTargetPosCurrent.aisdynamic().cogdegreex10()+turnRight*DegreesX10_ToTurn_WhenMeetLand;
            if(newCOGX10<0)
                newCOGX10+=3600;
            else
                newCOGX10%=3600;

            pbTargetPosOrig.mutable_aisdynamic()->set_cogdegreex10(newCOGX10);
            bool isNewPosOnLand;
            reckonPbTargetPosCurrentAndCalibrateCOG(currentDateTime,isNewPosOnLand);
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
            pbTargetPosOrig.CopyFrom(pbTargetPosCurrent);
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
    QMapIterator <PB_Enum_TargetInfo_Type, Struct_PosDeviceInfo> iMapInfoTypePosDeviceInfo(world->getMapInfoTypePosDeviceInfo());
    while(iMapInfoTypePosDeviceInfo.hasNext())
    {
        iMapInfoTypePosDeviceInfo.next();
        PB_Enum_TargetInfo_Type infoType= iMapInfoTypePosDeviceInfo.key();
        Struct_PosDeviceInfo posDevInfo=iMapInfoTypePosDeviceInfo.value();
        if(hashTargetInfoTypePosDevice.contains(infoType))
        {
            qDebug()<<"Warning: try to install already existed devices. Ignored.";
            continue;
        }
        PosDevice *posDev=new PosDevice(QDateTime::currentDateTime().addMSecs(-1*qrand()%posDevInfo.sampleMilliSeconds),
                                        this,infoType);
        hashTargetInfoTypePosDevice.insert(infoType,posDev);
    }
    return true;
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

PBTargetPosition Target::updateAndGetPbTargetPosCurrent()
{
    updateTargetPosCurrentAndOrigIfMeetLand();
    return pbTargetPosCurrent;
}

void Target::reckonPbTargetPosCurrentAndCalibrateCOG(const QDateTime &dtToReckon, bool &isOnLand)
{
    if(pbTargetPosOrig.aisdynamic().sogknotsx10()==0)
    {
        pbTargetPosCurrent.mutable_aisdynamic()->set_utctimestamp(dtToReckon.toTime_t());
        isOnLand=false;
        return;
    }

    pbTargetPosBeforeCurrent.CopyFrom(pbTargetPosCurrent);
    PBAISDynamic aisDynamicOrig=pbTargetPosOrig.aisdynamic();
    qint64 miliSecondsElapsed=posOrigDateTime.msecsTo(dtToReckon);
    double distance=aisDynamicOrig.sogknotsx10()/10.0*NM_In_Meter/3600.0*miliSecondsElapsed/1000.0;
    QGeoCoordinate geoOrig(aisDynamicOrig.intlatitudex60w()/AISPosDivider,aisDynamicOrig.intlongitudex60w()/AISPosDivider);
    QGeoCoordinate geoReckoned=geoOrig.atDistanceAndAzimuth(distance,aisDynamicOrig.cogdegreex10()/10.0,0);
    pbTargetPosCurrent.mutable_aisdynamic()->set_intlongitudex60w(geoReckoned.longitude()*AISPosDivider);
    pbTargetPosCurrent.mutable_aisdynamic()->set_intlatitudex60w(geoReckoned.latitude()*AISPosDivider);

    pbTargetPosCurrent.mutable_aisdynamic()->set_utctimestamp(dtToReckon.toTime_t());

    if(pbTargetPosBeforeCurrent.aisdynamic().utctimestamp()!=pbTargetPosCurrent.aisdynamic().utctimestamp())
    {
        QGeoCoordinate geoPosBeforeCurrent(pbTargetPosBeforeCurrent.aisdynamic().intlatitudex60w()/AISPosDivider,
                                           pbTargetPosBeforeCurrent.aisdynamic().intlongitudex60w()/AISPosDivider);
        float newCOGInDegree=geoPosBeforeCurrent.azimuthTo(geoReckoned);
        pbTargetPosCurrent.mutable_aisdynamic()->set_cogdegreex10(qRound(newCOGInDegree*10));
        pbTargetPosCurrent.mutable_aisdynamic()->set_headingdegree(qRound(newCOGInDegree));
    }

    if(world->isInWater(geoReckoned.longitude(),geoReckoned.latitude()))
    {
        isOnLand=false;
    }
    else
        isOnLand=true;
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
