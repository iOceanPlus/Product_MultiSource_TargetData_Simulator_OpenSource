#include "PBCoderDecoder.h"
#include <QDebug>
#include <QDateTime>

//#include "macro.h"
PBCoderDecoder::PBCoderDecoder(PB_Enum_Software enum_SoftwareName, QObject *parent) : QObject(parent)
{
    serialNum=0;
    this->pbEnumSenderSoftware = enum_SoftwareName;

    startedTimeUTC=QDateTime::currentDateTime().toTime_t();
    // Verify that the version of the library that we linked against is
    // compatible with the version of the headers we compiled against.
    GOOGLE_PROTOBUF_VERIFY_VERSION;
}

PBCoderDecoder::~PBCoderDecoder()
{

}

QString PBCoderDecoder::getReadableTargetInfo_SourceName(const PB_Enum_TargetInfo_Source &targetInfoSource)
{
    switch (targetInfoSource) {
    case EV_TargetInfoSource_AgricultureMinistry:
        return "农业部";
    case EV_TargetInfoSource_MaritimeBureau:
        return "海事局";
    case EV_TargetInfoSource_Haijian:
        return "Haijian";
    case EV_TargetInfoSource_SatAIS:
        return "卫星AIS";
    case EV_TargetInfoSource_SearchAndRescue:
        return "救捞局";
    default:
        return "其他";
    }
}

QString PBCoderDecoder::getReadableTargetInfo_TypeName(const PB_Enum_TargetInfo_Type &targetinfoTypeName)
{
    switch (targetinfoTypeName) {
    case EV_TargetInfoType_AISDynamic:
        return "AIS";
    case EV_TargetInfoType_AISStatic:
        return "AIS";
    case EV_TargetInfoType_ArgosAndMaritimeSatellite:
        return "Arogs及海事卫星";
    case EV_TargetInfoType_Beidou:
        return "北斗";
    case EV_TargetInfoType_Haijian:
        return "船舶";
    case EV_TargetInfoType_LRIT:
        return "LRIT";
    default:
        return "其他";
    }
}

PB_Enum_Aggregated_AIS_Ship_Type  PBCoderDecoder::getAggregatedAISShipType(qint16 shipTypeAIS)
{
    if(shipTypeAIS<20||shipTypeAIS>=90)
        return EV_AggregatedAISShipType_Others;
    else if(shipTypeAIS>=70&&shipTypeAIS<=79)
        return EV_AggregatedAISShipType_CargoShip;
    else if(shipTypeAIS==51)
        return EV_AggregatedAISShipType_SARShip;
    else if(shipTypeAIS>=80&&shipTypeAIS<=89)
        return EV_AggregatedAISShipType_Tanker;
    else if(shipTypeAIS==52)
        return EV_AggregatedAISShipType_Tug;
    else if(shipTypeAIS==30)
        return EV_AggregatedAISShipType_FishingShip;
    else if(shipTypeAIS==31||shipTypeAIS==32)
        return EV_AggregatedAISShipType_TowingShip;
    else if(shipTypeAIS>=60&&shipTypeAIS<=69)
        return EV_AggregatedAISShipType_PassengerShip;
    else if(shipTypeAIS==35)
        return EV_AggregatedAISShipType_MilitaryShip;
    else if(shipTypeAIS==55)
        return EV_AggregatedAISShipType_LawEnforcementShip;
    else if(shipTypeAIS>=20&&shipTypeAIS<=29)
        return EV_AggregatedAISShipType_WIGShip;
    else if(shipTypeAIS>=40&&shipTypeAIS<=49)
        return EV_AggregatedAISShipType_HSCShip;
    else if(shipTypeAIS==50)
        return EV_AggregatedAISShipType_Pilot;
    else if(shipTypeAIS==53)
        return EV_AggregatedAISShipType_PortTender;
    else if(shipTypeAIS==36)
        return EV_AggregatedAISShipType_Sailing;
    else if(shipTypeAIS==37)
        return EV_AggregatedAISShipType_PleasureCraft;
    else if(shipTypeAIS==33)
        return EV_AggregatedAISShipType_UnderwaterOperation;
    else if(shipTypeAIS==34)
        return EV_AggregatedAISShipType_Diving;
    else
        return EV_AggregatedAISShipType_Others;

}


quint32 PBCoderDecoder::getStartedTimeUTC() const
{
    return startedTimeUTC;
}

void PBCoderDecoder::setStartedTimeUTC(const quint32 &value)
{
    startedTimeUTC = value;
}

PB_Enum_Software PBCoderDecoder::getPbEnumSenderSoftware() const
{
    return pbEnumSenderSoftware;
}

void PBCoderDecoder::setPbEnumSenderSoftware(const PB_Enum_Software &value)
{
    pbEnumSenderSoftware = value;
}

quint32 PBCoderDecoder::getSerialNumAndIncrement()
{
    quint32 result=serialNum;
    serialNum++;
    return result;
}

QByteArray PBCoderDecoder::serializePBTargetToArray(PBTarget pbTarget)
{
    QByteArray baResult;
    baResult.resize(pbTarget.ByteSize());
    pbTarget.SerializeToArray(baResult.data(),pbTarget.ByteSize());
    return baResult;
}

