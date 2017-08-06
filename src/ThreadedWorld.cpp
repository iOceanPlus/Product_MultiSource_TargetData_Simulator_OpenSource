#include "ThreadedWorld.h"
#include <QMutex>
#include <QDebug>
#include <QTimer>
#include <QtMath>
#include <exception>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QTimer>
#include <QStringListIterator>
#include <QStringList>
#include "macro.h"
#include "target.h"

ThreadedWorld::ThreadedWorld(QMutex *mutex, MyQtGeoPolygon *geoPolyGonBoundingRegion, PBCoderDecoder *pbCodDecoder,
                             quint16 worldIndex,  QObject *parent) :   QObject(parent)
{
    this->sharedGeoPolyGonBoundingRegion=geoPolyGonBoundingRegion;
    this->wordIndex=worldIndex;
    this->sharedMutex=mutex;

    this->sharedPbCoderDecoder=pbCodDecoder;

    timerMeasureAndUpdateTargetPos=new QTimer(this);
    connect(timerMeasureAndUpdateTargetPos,&QTimer::timeout,this, &ThreadedWorld::slotTimerEventMeasureAndUpdateTargetsPos);
    timerMeasureAndUpdateTargetPos->start(ExternV_Milliseconds_FetchData);

    timerOutputTargetCountAndMsgRate=new QTimer(this);
    connect(timerOutputTargetCountAndMsgRate, &QTimer::timeout,this,&ThreadedWorld::slotTimerEventOutPutTargetCountAndMsgRate);
    timerOutputTargetCountAndMsgRate->start(ExternV_SECONDS_CHECK_TARGET_COUNT*1000);
}

ThreadedWorld::~ThreadedWorld()
{
    QHashIterator <qint32, Target*> iHashTargets(hashIDTarget);
    while (iHashTargets.hasNext())
    {
       iHashTargets.next();
       Target *target=iHashTargets.value();
        if(target)
            delete target;
    }
}

void ThreadedWorld::slotTimerEventMeasureAndUpdateTargetsPos()
{
#ifdef DEBUG_PERFORMANCE
    QTime time;
    time.start();
#endif

    /**************************************** put data to channels*********************/
    QMapIterator <PB_Enum_TargetInfo_Type,DataChannel*> iMapInfoTypeDataChannels(mapInfoTypeDataChannels);
    while(iMapInfoTypeDataChannels.hasNext())
    {
        iMapInfoTypeDataChannels.next();
        iMapInfoTypeDataChannels.value()->fetchDataFromPosDevicesIntoChannel();
    }

#ifdef DEBUG_PERFORMANCE
    qDebug()<<"Milliseconds to fetch data from pos devices:"<<  time.elapsed();
    time.start();
#endif

    /**********************************Data sources get data from channels****************/
    QMapIterator <PB_Enum_TargetInfo_Source,DataSource*> iMapInfoSourceDataSources(mapInfoSourceDataSources);
    while(iMapInfoSourceDataSources.hasNext())
    {
        iMapInfoSourceDataSources.next();
        iMapInfoSourceDataSources.value()->fetchDataFromChannelsAndSendToMQ();
    }

#ifdef DEBUG_PERFORMANCE
    qDebug()<<"Milliseconds to fetch data from Channels:"<<  time.elapsed();
    time.start();
#endif

    /*************************Clear data in data channels*****************************/
    iMapInfoTypeDataChannels.toFront();
    while(iMapInfoTypeDataChannels.hasNext())
    {
        iMapInfoTypeDataChannels.next();
        iMapInfoTypeDataChannels.value()->clearListPBTargetPosInChannel();
    }
}

void ThreadedWorld::slotTimerEventOutPutTargetCountAndMsgRate(QSet <qint32> &setDistinctTargetIDOrig)
{
    qint32 targetCountSumAccordingToInfoTypeAndOrigTargetID=0;
    qint32 targetCountAll=0;
    quint64 messageCountSum=0;
    float msgCountPerMinuteCount=0;

#ifdef DEBUG_PERFORMANCE
    QTime time;
    time.start();
#endif

    QMapIterator <PB_Enum_TargetInfo_Source,DataSource*> iMapInfoSourceDataSources(mapInfoSourceDataSources);
    while(iMapInfoSourceDataSources.hasNext())
    {
        iMapInfoSourceDataSources.next();
        targetCountAll+=iMapInfoSourceDataSources.value()->getTotalTargetCount();
        msgCountPerMinuteCount+=iMapInfoSourceDataSources.value()->getposCountPerMinute();
        messageCountSum+=iMapInfoSourceDataSources.value()->getTotalPosCountFetched();
        iMapInfoSourceDataSources.value()->uniteSetTargetID(mapInfoTypeSetTargetID);
        iMapInfoSourceDataSources.value()->uniteSetDistinctOrigTargetID(setDistinctTargetIDOrig);
    }
    std::cout<< QDateTime::currentDateTime().toString("MM/dd hh:mm:ss").toStdString()<<"\t各数据源消息率总计:"<<
                QString::number(msgCountPerMinuteCount,'g',3).toStdString()<<"/分钟\t发送的总轨迹点数:"<<messageCountSum;

    QMapIterator <PB_Enum_TargetInfo_Type,QSet <qint32>> iMapInfoTypeSetTargetID(mapInfoTypeSetTargetID);
    while(iMapInfoTypeSetTargetID.hasNext())
    {
        iMapInfoTypeSetTargetID.next();
        qint32 setSize=iMapInfoTypeSetTargetID.value().size();
        //qDebug()<<PBCoderDecoder::getReadableTargetInfo_TypeName(iMapInfoTypeSetTargetID.key())<<iMapInfoTypeSetTargetID.value();
        targetCountSumAccordingToInfoTypeAndOrigTargetID+=setSize;
        std::cout<<"\t"<<PBCoderDecoder::getReadableTargetInfo_TypeName(iMapInfoTypeSetTargetID.key()).toStdString()<<"目标总数:"<<setSize;
    }

    std::cout<<"\t各数据源不同原始编号目标总数(去重):"<<setDistinctTargetIDOrig.size()<<
               "\t<信息类型-目标原始ID>组合数量："<<targetCountSumAccordingToInfoTypeAndOrigTargetID<<
               "\t各数据源目标数的和(不去重):"<<targetCountAll<<endl;

#ifdef DEBUG_TargetCount
    qDebug()<<"mapInfoTypeOrigTargetIDForDebug size is:"<<multiMapInfoTypeOrigTargetIDForDebug.size()<<". Contents are:"<<multiMapInfoTypeOrigTargetIDForDebug;
#endif

#ifdef DEBUG_PERFORMANCE
    qDebug()<<"Milliseconds to output target count:"<<  time.elapsed();
    time.start();
#endif
}

 bool ThreadedWorld::addDataSourceIfNotExist(const PB_Enum_TargetInfo_Source &pbTargetInfoSource,
                              const QMap <PB_Enum_TargetInfo_Type,Struct_TransmissionQuality>  &mapInfoTypeTransmitQuality)
 {
     if(!mapInfoSourceDataSources.contains(pbTargetInfoSource))
     {
         DataSource *dataSource=new DataSource(this,pbTargetInfoSource, mapInfoTypeTransmitQuality,this);
          connect(dataSource, &DataSource::sigSend2MQ,this,&ThreadedWorld::sigSend2MQ);
          mapInfoSourceDataSources.insert(pbTargetInfoSource,dataSource);
          return true;
     }
     else
        return false;
 }

bool ThreadedWorld::createOneTarget(qint32 &targetID, const PBTargetPosition &pbTargetPos, const QDateTime &posOrigDateTime)
{
    Target *target=new Target(pbTargetPos,this,posOrigDateTime);
    target->installPosDevices();
    hashIDTarget.insert(targetID,target);

    QMapIterator <PB_Enum_TargetInfo_Source,DataSource*> iMapInfoSourceDataSources(mapInfoSourceDataSources);
    while(iMapInfoSourceDataSources.hasNext())
    {
        iMapInfoSourceDataSources.next();
        DataSource *dataSource=iMapInfoSourceDataSources.value();
         QMap <PB_Enum_TargetInfo_Type,Struct_TransmissionQuality> mapInfoTypeTransmitQuality=
                 dataSource->getMapInfoTypeTransmitQuality();
         if(mapInfoTypeTransmitQuality.contains(EV_TargetInfoType_AISDynamic))
         {
            Struct_TransmissionQuality  transQual= mapInfoTypeTransmitQuality.value(EV_TargetInfoType_AISDynamic);
            if(qrand()%100<transQual.percentageTargetsObserved)
                dataSource->addTargetIDObservedWithAIS(targetID);
         }
         if(mapInfoTypeTransmitQuality.contains(EV_TargetInfoType_LRIT))
         {
            Struct_TransmissionQuality  transQual= mapInfoTypeTransmitQuality.value(EV_TargetInfoType_LRIT);
            if(qrand()%100<transQual.percentageTargetsObserved)
                dataSource->addTargetIDObservedWithLRIT(targetID);
         }
         if(mapInfoTypeTransmitQuality.contains(EV_TargetInfoType_Beidou))
         {
            Struct_TransmissionQuality  transQual= mapInfoTypeTransmitQuality.value(EV_TargetInfoType_Beidou);
            if(qrand()%100<transQual.percentageTargetsObserved)
                dataSource->addTargetIDObservedWithBeidou(targetID);
         }
         if(mapInfoTypeTransmitQuality.contains(EV_TargetInfoType_Haijian))
         {
            Struct_TransmissionQuality  transQual= mapInfoTypeTransmitQuality.value(EV_TargetInfoType_Haijian);
            if(qrand()%100<transQual.percentageTargetsObserved)
                dataSource->addTargetIDObservedWithHaijian(targetID);
         }
         if(mapInfoTypeTransmitQuality.contains(EV_TargetInfoType_ArgosAndMaritimeSatellite))
         {
            Struct_TransmissionQuality  transQual= mapInfoTypeTransmitQuality.value(EV_TargetInfoType_ArgosAndMaritimeSatellite);
            if(qrand()%100<transQual.percentageTargetsObserved)
                dataSource->addTargetIDObservedWithArgosAndMarineSat(targetID);
         }
    }
    return true;
 }

 void ThreadedWorld::initDataChannels(const QMap <PB_Enum_TargetInfo_Type, Struct_PosDeviceInfo> &mapInfoTypePosDeviceInfo)
 {
     if(mapInfoTypeDataChannels.isEmpty())
     {
         QMapIterator <PB_Enum_TargetInfo_Type, Struct_PosDeviceInfo> iMapInfoTypePosDeviceInfo(mapInfoTypePosDeviceInfo);
         while(iMapInfoTypePosDeviceInfo.hasNext())
         {
             iMapInfoTypePosDeviceInfo.next();
             DataChannel *dataChannel=new DataChannel(this,iMapInfoTypePosDeviceInfo.key(),this);
             mapInfoTypeDataChannels.insert(iMapInfoTypePosDeviceInfo.key(),dataChannel);
         }
     }
 }
 
 PBCoderDecoder *ThreadedWorld::getPbCoderDecoder() const
 {
     return sharedPbCoderDecoder;
 }
 
 QMap<PB_Enum_TargetInfo_Type, DataChannel *> ThreadedWorld::getMapInfoTypeDataChannels() const
 {
     return mapInfoTypeDataChannels;
 }
 
 QMap<PB_Enum_TargetInfo_Type, Struct_PosDeviceInfo> ThreadedWorld::getMapInfoTypePosDeviceInfo() const
 {
     return mapInfoTypePosDeviceInfo;
 }

 QHash<qint32, Target *> ThreadedWorld::getHashIDTarget() const
 {
     return hashIDTarget;
 }

