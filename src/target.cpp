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

    geoCurrentHighPreci.setLatitude(pbTargetPos.aisdynamic().intlatitudex60w()/AISPosDivider);
    geoCurrentHighPreci.setLongitude(pbTargetPos.aisdynamic().intlongitudex60w()/AISPosDivider);
    geoOrigHighPreci=geoBeforeCurrentHighPreci=geoCurrentHighPreci;
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
    if(pbTargetPos.enum_targetidorig_type()!=EV_TargetIDType_MMSI&& pbTargetPos.enum_targetidorig_type()!=EV_TargetIDType_IMO)
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
        pbTargetPosToSet.set_enum_targetidorig_type(EV_TargetIDType_MMSI);
        pbTargetPosToSet.set_targetidorig(pbTargetPosToSet.aisdynamic().mmsi());
        break;
    case EV_TargetInfoType_LRIT:
        pbTargetPosToSet.set_enum_targetidorig_type(EV_TargetIDType_IMO);
        pbTargetPosToSet.set_targetidorig(pbTargetPosToSet.aisstatic().imo());
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
    bool isOnLand;
    QDateTime currentDateTime=QDateTime::currentDateTime();
    if(posCurrentDateTime.msecsTo(currentDateTime)<MIN_Sample_MSEC)
    {
        return pbTargetPosCurrent;
    }
    if(pbTargetPosOrig.aisdynamic().sogknotsx10()<=0)
    {
         posCurrentDateTime=currentDateTime;
         pbTargetPosCurrent.mutable_aisdynamic()->set_utctimestamp(currentDateTime.toTime_t());
         return pbTargetPosCurrent;
    }

    QGeoCoordinate geoReckoned= getConstCurrentGeoPosHighPreciReckoned(geoOrigHighPreci,posOrigDateTime,pbTargetPosOrig.aisdynamic().sogknotsx10(),
                                                                   pbTargetPosOrig.aisdynamic().cogdegreex10()/10.0, currentDateTime, isOnLand);
    if(isOnLand) //find a new direction
    {
        pbTargetPosOrig=pbTargetPosBeforeCurrent=pbTargetPosCurrent ;
         posOrigDateTime=posCurrentDateTime;
        geoOrigHighPreci= geoBeforeCurrentHighPreci=geoCurrentHighPreci;

        bool turnRight=qrand()%2==0;
        bool newCOGGot=false;
        for(int i=1;i<=7;i++)
        {
            qint32 factor=turnRight?1:-1;
            qint32 newCOGX10=(qint32)pbTargetPosOrig.aisdynamic().cogdegreex10()+(qint32)factor*i*DegreesX10_ToTurn_WhenMeetLand;
            if(newCOGX10<0)
                newCOGX10+=3600;
            else
                newCOGX10%=3600;

            bool isNewPosOnLand;
            QGeoCoordinate geoReckonedTrial= getConstCurrentGeoPosHighPreciReckoned(geoOrigHighPreci,posOrigDateTime,pbTargetPosOrig.aisdynamic().sogknotsx10(),
                                                                           newCOGX10/10.0, currentDateTime, isNewPosOnLand);
            if(isNewPosOnLand)
                continue;
            else
            {
                newCOGGot=true;
                pbTargetPosOrig.mutable_aisdynamic()->set_cogdegreex10(newCOGX10);
                pbTargetPosBeforeCurrent.mutable_aisdynamic()->set_cogdegreex10(newCOGX10);
                pbTargetPosCurrent.mutable_aisdynamic()->set_cogdegreex10(newCOGX10);
                updatePosAndCOG(currentDateTime,geoReckonedTrial);
                break;
            }
        }
        if(!newCOGGot) //stop it
        {
            pbTargetPosOrig.mutable_aisdynamic()->set_sogknotsx10(0);
            pbTargetPosBeforeCurrent.mutable_aisdynamic()->set_sogknotsx10(0);
            pbTargetPosCurrent.mutable_aisdynamic()->set_sogknotsx10(0);
            pbTargetPosCurrent.mutable_aisdynamic()->set_utctimestamp(currentDateTime.toTime_t());
        }
    }
    else //not on land
    {
        updatePosAndCOG(currentDateTime,geoReckoned);
    }
    return pbTargetPosCurrent;
}

void Target::updatePosAndCOG(const QDateTime &dtReckoned, const QGeoCoordinate &geoReckoned)
{
    geoBeforeCurrentHighPreci=geoCurrentHighPreci;
    geoCurrentHighPreci=geoReckoned;

    pbTargetPosBeforeCurrent.CopyFrom(pbTargetPosCurrent);
    posCurrentDateTime=dtReckoned;

    if(geoBeforeCurrentHighPreci!=geoCurrentHighPreci)
    {
        float newCOGInDegree=geoBeforeCurrentHighPreci.azimuthTo(geoReckoned);
        pbTargetPosCurrent.mutable_aisdynamic()->set_headingdegree(qRound(newCOGInDegree));
        pbTargetPosCurrent.mutable_aisdynamic()->set_cogdegreex10(qRound(newCOGInDegree*10));
    }
    pbTargetPosCurrent.mutable_aisdynamic()->set_intlongitudex60w(qRound(geoReckoned.longitude()*AISPosDivider));
    pbTargetPosCurrent.mutable_aisdynamic()->set_intlatitudex60w( qRound(geoReckoned.latitude()*AISPosDivider));
    pbTargetPosCurrent.mutable_aisdynamic()->set_utctimestamp(dtReckoned.toTime_t());
}

const QGeoCoordinate Target::getConstCurrentGeoPosHighPreciReckoned(const QGeoCoordinate &geoOrig, const QDateTime &dtOrig,
                                             const double &sogKnotsX10,   const double &degreeAziumth, const QDateTime &dtToReckon, bool &isOnLand) const
{
    if(sogKnotsX10<=0)
    {
        isOnLand=false;
        return geoOrig;
    }

    qint64 miliSecondsElapsed=dtOrig.msecsTo(dtToReckon);
    QGeoCoordinate geoReckoned;
    if(miliSecondsElapsed>0)
    {
        double distance=sogKnotsX10/10.0*NM_In_Meter/3600.0*miliSecondsElapsed/1000.0;
        geoReckoned=geoOrig.atDistanceAndAzimuth(distance,degreeAziumth,0);
    }
    else
        geoReckoned=geoOrig;

    if(world->isInWater(geoReckoned.longitude(),geoReckoned.latitude()))
    {
        isOnLand=false;
    }
    else
        isOnLand=true;

    return geoReckoned;
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
