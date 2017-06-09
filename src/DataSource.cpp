#include "DataSource.h"
#include "ParallelWorld.h"
#include "PBCoderDecoder.h"
#include <QDebug>

DataSource::DataSource(ParallelWorld *world, const PB_Enum_TargetInfo_Source &pbTargetInfoSource,
                       const QMap <PB_Enum_TargetInfo_Type,Struct_TransmissionQuality>  &mapInfoTypeTransmitQuality,
                       QObject *parent ) : QObject(parent)
{
    this->mapInfoTypeTransmitQuality=mapInfoTypeTransmitQuality;
    this->world=world;
    this->pbTargetInfoSource=pbTargetInfoSource;

    totalPosCountFetched=posCountOutputToLog= 0;
    dtPosCountFetched=dtPosCountOutputToLog= QDateTime::currentDateTime();
}

bool DataSource::addTargetIDObservedWithAIS(qint32 targetID)
{
    setTargetIDsObservedWithAIS.insert(targetID);
    return true;
}

bool DataSource::addTargetIDObservedWithBeidou(qint32 targetID)
{
    setTargetIDsObservedWithBeidou.insert(targetID);
    return true;
}

bool DataSource::addTargetIDObservedWithArgosAndMarineSat(qint32 targetID)
{
    setTargetIDsObservedWithArgosAndMarineSat.insert(targetID);
    return true;
}

bool DataSource::addTargetIDObservedWithHaijian(qint32 targetID)
{
    setTargetIDsObservedWithHaijian.insert(targetID);
    return true;
}

bool DataSource::fetchDataFromChannelsAndSendToMQ()
{
    QList <StructDataAndKey> listDataAndKey;

    if(!setTargetIDsObservedWithAIS.isEmpty()&&mapInfoTypeTransmitQuality.contains(EV_TargetInfoType_AISDynamic)
            &&world->getMapInfoTypeDataChannels().contains(EV_TargetInfoType_AISDynamic))
    {
        fetchDataFromAChannel(EV_TargetInfoType_AISDynamic,listDataAndKey,setTargetIDsObservedWithAIS,ROUTING_KEY_ONLINE_PREPROCESSED_AIS);
    }

    if(!setTargetIDsObservedWithBeidou.isEmpty()&&mapInfoTypeTransmitQuality.contains(EV_TargetInfoType_Beidou)
            &&world->getMapInfoTypeDataChannels().contains(EV_TargetInfoType_Beidou))
    {
        fetchDataFromAChannel(EV_TargetInfoType_Beidou,listDataAndKey,setTargetIDsObservedWithBeidou,ROUTING_KEY_ONLINE_PREPROCESSED_Beidou);
    }

    if(!setTargetIDsObservedWithHaijian.isEmpty()&&mapInfoTypeTransmitQuality.contains(EV_TargetInfoType_Haijian)
            &&world->getMapInfoTypeDataChannels().contains(EV_TargetInfoType_Haijian))
    {
        fetchDataFromAChannel(EV_TargetInfoType_Haijian,listDataAndKey,setTargetIDsObservedWithHaijian,ROUTING_KEY_ONLINE_PREPROCESSED_Haijian);
    }

    if(!setTargetIDsObservedWithArgosAndMarineSat.isEmpty()&&mapInfoTypeTransmitQuality.contains(EV_TargetInfoType_ArgosAndMaritimeSatellite)
            &&world->getMapInfoTypeDataChannels().contains(EV_TargetInfoType_ArgosAndMaritimeSatellite))
    {
        fetchDataFromAChannel(EV_TargetInfoType_ArgosAndMaritimeSatellite,listDataAndKey,setTargetIDsObservedWithArgosAndMarineSat,ROUTING_KEY_ONLINE_PREPROCESSED_Argos);
    }
    if(!listDataAndKey.isEmpty())
        emit sigSend2MQ(listDataAndKey);

    return true;
}

bool DataSource::fetchDataFromAChannel(const PB_Enum_TargetInfo_Type &targetInfoType, QList <StructDataAndKey> &listDataAndKey,
                                       QSet <qint32> &setTargetIDObservedOfThisInfoType, const QString &routingKey)
{
    Struct_TransmissionQuality transmissionQ=mapInfoTypeTransmitQuality.value(targetInfoType);

    PBTarget pbTarget;
    pbTarget.set_sequencenum(world->getPbCoderDecoder()->getSerialNumAndIncrement());
    pbTarget.set_enum_sender_software(world->getPbCoderDecoder()->getPbEnumSenderSoftware());

    QListIterator <PBTargetPosition> iListTargetPos(world->getMapInfoTypeDataChannels().value(targetInfoType)->getListPBTargetPosInChannel());
    while(iListTargetPos.hasNext())
    {
        PBTargetPosition pbTargetPosInList= iListTargetPos.next();
        if(!setTargetIDObservedOfThisInfoType.contains(pbTargetPosInList.targetid())
                ||qrand()%100<transmissionQ.packetLossPercentage )
            continue;

        addTimeStampErrorInDynamicOfTargetPos(pbTargetPosInList,transmissionQ);
        PBTargetPosition *pbTargetPosToAdd= pbTarget.add_listtargetpos();
        pbTargetPosToAdd->CopyFrom(pbTargetPosInList);
    }
    if(pbTarget.listtargetpos_size()>0)
    {
        StructDataAndKey dataAndKey;
        dataAndKey.data= world->getPbCoderDecoder()->serializePBTargetToArray(pbTarget);
        dataAndKey.routingKey=routingKey;
        listDataAndKey.append(dataAndKey);
    }
    return true;
}

void DataSource::addTimeStampErrorInDynamicOfTargetPos(PBTargetPosition &pbTargetPos, const Struct_TransmissionQuality &transQ) const
{
    if(transQ.stdDevTimestampErrorInMiliSeconds>0)
        pbTargetPos.mutable_aisdynamic()->set_utctimestamp(pbTargetPos.aisdynamic().utctimestamp()+
                                   qRound( (transQ.meanTimestampErrorInMiliSeconds+qrand()%transQ.stdDevTimestampErrorInMiliSeconds)/1000.0) );
    else
        pbTargetPos.mutable_aisdynamic()->set_utctimestamp(pbTargetPos.aisdynamic().utctimestamp()+
                                   qRound(transQ.meanTimestampErrorInMiliSeconds/1000.0) );
}

void DataSource::slotOutPutTargetsCountPerType()
{







}

void DataSource::slotOutPutPosCountAndRate()
{
    qint64 newPosCount=totalPosCountFetched-posCountOutputToLog;
    qint32 posCountPerMinute=qRound(newPosCount*1.00000/dtPosCountOutputToLog.msecsTo(dtPosCountFetched)*1000*60);
    qDebug()<<PBCoderDecoder::getReadableTargetInfo_SourceName(pbTargetInfoSource)<<":"<<




    posCountOutputToLog=totalPosCountFetched;
   dtPosCountOutputToLog=dtPosCountFetched;
}

QMap<PB_Enum_TargetInfo_Type, Struct_TransmissionQuality> DataSource::getMapInfoTypeTransmitQuality() const
{
    return mapInfoTypeTransmitQuality;
}

PB_Enum_TargetInfo_Source DataSource::getPbTargetInfoSource() const
{
    return pbTargetInfoSource;
}
