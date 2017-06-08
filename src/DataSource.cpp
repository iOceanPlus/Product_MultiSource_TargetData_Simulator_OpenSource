#include "DataSource.h"
#include "ParallelWorld.h"
#include "PBCoderDecoder.h"

DataSource::DataSource(ParallelWorld *world, const PB_Enum_TargetInfo_Source &pbTargetInfoSource,
                       const QMap <PB_Enum_TargetInfo_Type,Struct_TransmissionQuality>  &mapInfoTypeTransmitQuality,
                       QObject *parent ) : QObject(parent)
{
    this->mapInfoTypeTransmitQuality=mapInfoTypeTransmitQuality;
    this->world=world;
    this->pbTargetInfoSource=pbTargetInfoSource;
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
        Struct_TransmissionQuality transmissionQ=mapInfoTypeTransmitQuality.value(EV_TargetInfoType_AISDynamic);

        PBTarget pbTarget;
        pbTarget.set_sequencenum(world->getPbCoderDecoder()->getSerialNumAndIncrement());
        pbTarget.set_enum_sender_software(world->getPbCoderDecoder()->getPbEnumSenderSoftware());

        QListIterator <PBTargetPosition> iListTargetPos(world->getMapInfoTypeDataChannels().value(EV_TargetInfoType_AISDynamic)->getListPBTargetPosInChannel());
        while(iListTargetPos.hasNext())
        {
            PBTargetPosition pbTargePos= iListTargetPos.next();
            if(!setTargetIDsObservedWithAIS.contains(pbTargePos.targetid())
                    ||qrand()%100<transmissionQ.packetLossPercentage )
                continue;

            addTimeStampErrorInDynamicOfTargetPos(pbTargePos,transmissionQ);



           }
        if(pbTarget.listtargetpos_size()>0)
        {


        }

    }






    if(!listDataAndKey.isEmpty())
        emit sigSend2MQ(listDataAndKey);

    return true;
}

void DataSource::addTimeStampErrorInDynamicOfTargetPos(PBTargetPosition &pbTargetPos, Struct_TransmissionQuality transQ) const
{
    pbTargetPos.mutable_aisdynamic()->set_utctimestamp(pbTargetPos.aisdynamic().utctimestamp()+
                                   qRound( (transQ.meanTimestampErrorInMiliSeconds+qrand()%transQ.stdDevTimestampErrorInMiliSeconds)/1000.0) );

}

QMap<PB_Enum_TargetInfo_Type, Struct_TransmissionQuality> DataSource::getMapInfoTypeTransmitQuality() const
{
    return mapInfoTypeTransmitQuality;
}

PB_Enum_TargetInfo_Source DataSource::getPbTargetInfoSource() const
{
    return pbTargetInfoSource;
}
