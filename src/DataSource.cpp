#include "DataSource.h"
#include "World.h"
#include "PBCoderDecoder.h"
#include <QDebug>
#include <QTimer>

DataSource::DataSource(World *world, const PB_Enum_TargetInfo_Source &pbTargetInfoSource,
                       const QMap <PB_Enum_TargetInfo_Type,Struct_TransmissionQuality>  &mapInfoTypeTransmitQuality,
                       QObject *parent ) : QObject(parent)
{
    this->mapInfoTypeTransmitQuality=mapInfoTypeTransmitQuality;
    this->world=world;
    this->pbTargetInfoSource=pbTargetInfoSource;

    posCountPerMinute=0;

    totalPosCountFetched=posCountOutputToLog= 0;
    dtPosCountFetched=dtPosCountOutputToLog= QDateTime::currentDateTime();
    timerOutPutInfo=new QTimer(this);
    connect(timerOutPutInfo,&QTimer::timeout,this,&DataSource::slotOutPutTargetsCountPerType);
    connect(timerOutPutInfo,&QTimer::timeout,this,&DataSource::slotOutPutPosCountAndRate);
    timerOutPutInfo->start(ExternV_SECONDS_CHECK_TARGET_COUNT*1000);
}

bool DataSource::addTargetIDObservedWithAIS(qint32 targetID)
{
    setTargetIDsObservedWithAIS.insert(targetID);
    return true;
}

bool DataSource::addTargetIDObservedWithLRIT(qint32 targetID)
{
    setTargetIDsObservedWithLRIT.insert(targetID);
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
        fetchDataFromAChannelAndSendToMQ(EV_TargetInfoType_AISDynamic,listDataAndKey,setTargetIDsObservedWithAIS,setTargetIDsSentWithAIS,
                              ROUTING_KEY_ONLINE_PREPROCESSED_AIS);
    }

    if(!setTargetIDsObservedWithLRIT.isEmpty()&&mapInfoTypeTransmitQuality.contains(EV_TargetInfoType_LRIT)
            &&world->getMapInfoTypeDataChannels().contains(EV_TargetInfoType_LRIT))
    {
        fetchDataFromAChannelAndSendToMQ(EV_TargetInfoType_LRIT,listDataAndKey,setTargetIDsObservedWithLRIT,setTargetIDsSentWithLRIT,
                              ROUTING_KEY_ONLINE_PREPROCESSED_AIS);
    }

    if(!setTargetIDsObservedWithBeidou.isEmpty()&&mapInfoTypeTransmitQuality.contains(EV_TargetInfoType_Beidou)
            &&world->getMapInfoTypeDataChannels().contains(EV_TargetInfoType_Beidou))
    {
        fetchDataFromAChannelAndSendToMQ(EV_TargetInfoType_Beidou,listDataAndKey,setTargetIDsObservedWithBeidou,setTargetIDsSentWithBeidou,
                              ROUTING_KEY_ONLINE_PREPROCESSED_Beidou);
    }

    if(!setTargetIDsObservedWithHaijian.isEmpty()&&mapInfoTypeTransmitQuality.contains(EV_TargetInfoType_Haijian)
            &&world->getMapInfoTypeDataChannels().contains(EV_TargetInfoType_Haijian))
    {
        fetchDataFromAChannelAndSendToMQ(EV_TargetInfoType_Haijian,listDataAndKey,setTargetIDsObservedWithHaijian,setTargetIDsSentWithHaijian,
                              ROUTING_KEY_ONLINE_PREPROCESSED_Haijian);
    }

    if(!setTargetIDsObservedWithArgosAndMarineSat.isEmpty()&&mapInfoTypeTransmitQuality.contains(EV_TargetInfoType_ArgosAndMaritimeSatellite)
            &&world->getMapInfoTypeDataChannels().contains(EV_TargetInfoType_ArgosAndMaritimeSatellite))
    {
        fetchDataFromAChannelAndSendToMQ(EV_TargetInfoType_ArgosAndMaritimeSatellite,listDataAndKey,setTargetIDsObservedWithArgosAndMarineSat,
                              setTargetIDsSentWithArgosAndMarineSat,  ROUTING_KEY_ONLINE_PREPROCESSED_Argos);
    }

    return true;
}

bool DataSource::fetchDataFromAChannelAndSendToMQ(const PB_Enum_TargetInfo_Type &targetInfoType, QList <StructDataAndKey> &listDataAndKey,
                                       QSet <qint32> &setTargetIDObservedOfThisInfoType,QSet <qint32> &setTargetIDSentOfThisInfoType, const QString &routingKey)
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

        setOrigTargetIDs.insert(pbTargetPosInList.targetidorig());

        setTargetIDSentOfThisInfoType.insert(pbTargetPosInList.targetid());
#ifdef DEBUG_TargetCount
        if(!world->multiMapInfoTypeOrigTargetIDForDebug.contains(targetInfoType,pbTargetPosInList.targetidorig()))
            world->multiMapInfoTypeOrigTargetIDForDebug.insert(targetInfoType,pbTargetPosInList.targetidorig());
#endif

        addTimeStampErrorInDynamicOfTargetPos(pbTargetPosInList,transmissionQ);
        pbTargetPosInList.set_enum_targetinfosource(pbTargetInfoSource);

#ifdef DATA_SOURCE_RECODE_TARGETIDORIG
        qint32 infoSourceInfotypeTargetIDOrig=pbTargetInfoSource;
        infoSourceInfotypeTargetIDOrig=infoSourceInfotypeTargetIDOrig<<26;
        infoSourceInfotypeTargetIDOrig+= (qint32)pbTargetPosInList.enum_targetinfosource()<<22;
        infoSourceInfotypeTargetIDOrig+=pbTargetPosInList.targetidorig();

        pbTargetPosInList.set_targetidorig(infoSourceInfotypeTargetIDOrig);// Recode it
#endif

        PBTargetPosition *pbTargetPosToAdd= pbTarget.add_listtargetpos();
        pbTargetPosToAdd->CopyFrom(pbTargetPosInList);

       // qDebug()<<PBCoderDecoder::getReadableTargetInfo_TypeName(pbTargetPosToAdd->enum_targetinfotype())<<pbTargetPosToAdd->targetidorig();
    }
    totalPosCountFetched+=pbTarget.listtargetpos_size();
    dtPosCountFetched=QDateTime::currentDateTime();
    //qDebug()<<PBCoderDecoder::getReadableTargetInfo_SourceName(this->getPbTargetInfoSource())<<  PBCoderDecoder::getReadableTargetInfo_TypeName(targetInfoType)<<pbTarget.listtargetpos_size();
    if(pbTarget.listtargetpos_size()>0)
    {
        StructDataAndKey dataAndKey;
        dataAndKey.data= world->getPbCoderDecoder()->serializePBTargetToArray(pbTarget);
        dataAndKey.routingKey=routingKey;
        listDataAndKey.append(dataAndKey);

        world->addPreprocessedMsgsSendInMonitorProbeAck(pbTarget.listtargetpos_size());
    }

    if(  !listDataAndKey.isEmpty()&&transmissionQ.meanTransmissionLatencyInMiliSeconds<=0)
        emit sigSend2MQ(listDataAndKey);
    else if( !listDataAndKey.isEmpty())
        QTimer::singleShot(transmissionQ.meanTransmissionLatencyInMiliSeconds+qrand()%transmissionQ.stdDevTransmissionLatencyInMiliSeconds,
                            [this, &listDataAndKey] () { emit sigSend2MQ(listDataAndKey);}  );

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

quint64 DataSource::getTotalPosCountFetched() const
{
    return totalPosCountFetched;
}

void DataSource::slotOutPutTargetsCountPerType()
{
    std::cout<<endl<< QDateTime::currentDateTime().toString("MM/dd hh:mm:ss").toStdString()<<" "<<
               PBCoderDecoder::getReadableTargetInfo_SourceName(pbTargetInfoSource).toStdString();
    if(!setTargetIDsSentWithAIS.isEmpty())
            std::cout<<"\t"<<PBCoderDecoder::getReadableTargetInfo_TypeName(EV_TargetInfoType_AISDynamic).toStdString()<<"目标数:"<<setTargetIDsSentWithAIS.size();
    if(!setTargetIDsSentWithLRIT.isEmpty())
           std::cout<<"\t"<<PBCoderDecoder::getReadableTargetInfo_TypeName(EV_TargetInfoType_LRIT).toStdString()<<"目标数:"<<setTargetIDsSentWithLRIT.size();
    if(!setTargetIDsSentWithBeidou.isEmpty())
            std::cout<<"\t"<<PBCoderDecoder::getReadableTargetInfo_TypeName(EV_TargetInfoType_Beidou).toStdString()<<"目标数:"<<setTargetIDsSentWithBeidou.size();
    if(!setTargetIDsSentWithHaijian.isEmpty())
           std::cout<<"\t"<<PBCoderDecoder::getReadableTargetInfo_TypeName(EV_TargetInfoType_Haijian).toStdString()<<"目标数:"<<setTargetIDsSentWithHaijian.size();
    if(!setTargetIDsSentWithArgosAndMarineSat.isEmpty())
          std::cout<<"\t"<<PBCoderDecoder::getReadableTargetInfo_TypeName(EV_TargetInfoType_ArgosAndMaritimeSatellite).toStdString()<<"目标数:"<<setTargetIDsSentWithArgosAndMarineSat.size();

    std::cout<<endl;
}

void DataSource::slotOutPutPosCountAndRate()
{
    qint64 newPosCount=totalPosCountFetched-posCountOutputToLog;
    posCountPerMinute=newPosCount*1.00000/dtPosCountOutputToLog.msecsTo(dtPosCountFetched)*1000*60;
    std::cout<< QDateTime::currentDateTime().toString("MM/dd hh:mm:ss").toStdString()<<" "<<
               PBCoderDecoder::getReadableTargetInfo_SourceName(pbTargetInfoSource).toStdString()<<"总计:"
            <<QString::number(posCountPerMinute,'g',3).toStdString()<<" 轨迹点/分钟"<<"\t该数据源发送总点数:"<<totalPosCountFetched<<endl;
    posCountOutputToLog=totalPosCountFetched;
   dtPosCountOutputToLog=dtPosCountFetched;
}

qint32 DataSource::getTotalTargetCount()
{
    return setTargetIDsSentWithAIS.size()+ setTargetIDsSentWithLRIT.size()+ setTargetIDsSentWithArgosAndMarineSat.size()+setTargetIDsSentWithBeidou.size()
            +setTargetIDsSentWithHaijian.size();
}

float DataSource::getposCountPerMinute()
{
    return posCountPerMinute;
}

void DataSource::uniteSetTargetID(QMap <PB_Enum_TargetInfo_Type, QSet <qint32> > &mapInfoTypeSetTargetID) const
{
    if(!mapInfoTypeSetTargetID.contains(EV_TargetInfoType_AISDynamic)&&!mapInfoTypeSetTargetID.contains(EV_TargetInfoType_AISStatic))
        mapInfoTypeSetTargetID.insert(EV_TargetInfoType_AISDynamic,QSet <qint32> ());

    if(!mapInfoTypeSetTargetID.contains(EV_TargetInfoType_ArgosAndMaritimeSatellite))
        mapInfoTypeSetTargetID.insert(EV_TargetInfoType_ArgosAndMaritimeSatellite,QSet <qint32> ());

    if(!mapInfoTypeSetTargetID.contains(EV_TargetInfoType_Beidou))
        mapInfoTypeSetTargetID.insert(EV_TargetInfoType_Beidou,QSet <qint32> ());

    if(!mapInfoTypeSetTargetID.contains(EV_TargetInfoType_Haijian))
        mapInfoTypeSetTargetID.insert(EV_TargetInfoType_Haijian,QSet <qint32> ());

    if(!mapInfoTypeSetTargetID.contains(EV_TargetInfoType_LRIT))
        mapInfoTypeSetTargetID.insert(EV_TargetInfoType_LRIT,QSet <qint32> ());

    QMutableMapIterator <PB_Enum_TargetInfo_Type, QSet <qint32> > iMapInfoTypeSetTargetID(mapInfoTypeSetTargetID);
    while(iMapInfoTypeSetTargetID.hasNext())
    {
        iMapInfoTypeSetTargetID.next();
        switch (iMapInfoTypeSetTargetID.key()) {
        case EV_TargetInfoType_AISDynamic:
        case EV_TargetInfoType_AISStatic:
            iMapInfoTypeSetTargetID.value().unite(setTargetIDsSentWithAIS);
            break;
        case EV_TargetInfoType_ArgosAndMaritimeSatellite:
            iMapInfoTypeSetTargetID.value().unite(setTargetIDsSentWithArgosAndMarineSat);
            break;
        case EV_TargetInfoType_Beidou:
            iMapInfoTypeSetTargetID.value().unite(setTargetIDsSentWithBeidou);
            break;
        case EV_TargetInfoType_Haijian:
            iMapInfoTypeSetTargetID.value().unite(setTargetIDsSentWithHaijian);
            break;
        case EV_TargetInfoType_LRIT:
            iMapInfoTypeSetTargetID.value().unite(setTargetIDsSentWithLRIT);
            break;
        default:
            break;
        }
    }
}

void DataSource::uniteSetDistinctOrigTargetID(QSet <qint32> &setDistinctOrigTargetID) const
{
    setDistinctOrigTargetID.unite(setOrigTargetIDs);
}

QMap<PB_Enum_TargetInfo_Type, Struct_TransmissionQuality> DataSource::getMapInfoTypeTransmitQuality() const
{
    return mapInfoTypeTransmitQuality;
}

PB_Enum_TargetInfo_Source DataSource::getPbTargetInfoSource() const
{
    return pbTargetInfoSource;
}
