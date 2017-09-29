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
                             quint16 worldIndex, const QString &language,  QObject *parent) :   QObject(parent)
{
    this->sharedGeoPolyGonBoundingRegion=geoPolyGonBoundingRegion;
    this->worldIndex=worldIndex;
    this->sharedMutex=mutex;

    this->sharedPbCoderDecoder=pbCodDecoder;
    this->language=language;

    qsrand(worldIndex+1);
    randomEngine=new std::default_random_engine(worldIndex);

    timerMeasureAndUpdateTargetPos=new QTimer(this);
    connect(timerMeasureAndUpdateTargetPos,&QTimer::timeout,this, &ThreadedWorld::slotTimerEventMeasureAndUpdateTargetsPos);
    timerMeasureAndUpdateTargetPos->start(ExternV_Milliseconds_FetchData);
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
    qint64 currentDateTimeMSecs=QDateTime::currentDateTime().toMSecsSinceEpoch();
    QMapIterator <PB_Enum_TargetInfo_Type,DataChannel*> iMapInfoTypeDataChannels(mapInfoTypeDataChannels);
    while(iMapInfoTypeDataChannels.hasNext())
    {
        iMapInfoTypeDataChannels.next();
        iMapInfoTypeDataChannels.value()->fetchDataFromPosDevicesIntoChannel(currentDateTimeMSecs);
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

QString ThreadedWorld::getLanguage() const
{
    return language;
}

std::default_random_engine *ThreadedWorld::getRandomEngine() const
{
    return randomEngine;
}

void ThreadedWorld::updateTargetCountAndMsgRate(QSet <qint32> &setDistinctTargetIDOrig,
                                                QMap <PB_Enum_TargetInfo_Type,  QSet <qint32> > &mapInfoTypeSetTargetID,qint32 &targetCountAll,
                               float &msgCountPerMinuteCount, quint64 &messageCountSum)
{
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
#ifdef DEBUG_PERFORMANCE
//    qDebug()<<"Milliseconds to output target count:"<<  time.elapsed();
//    time.start();
#endif
}

QMap<PB_Enum_TargetInfo_Source, DataSource *> ThreadedWorld::getMapInfoSourceDataSources() const
{
    return mapInfoSourceDataSources;
}

bool ThreadedWorld::slotAddDataSourceIfNotExist(const PB_Enum_TargetInfo_Source &pbTargetInfoSource,
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

bool ThreadedWorld::slotCreateTargets(const QList<PBTargetPosition> &listPbTargetPos, const quint16 &worldCount)
{
    QListIterator <PBTargetPosition> iListTargetPos(listPbTargetPos);
    qint64 timeInt64=QDateTime::currentDateTime().toMSecsSinceEpoch();
    while(iListTargetPos.hasNext())
    {
        PBTargetPosition pbTargetPos=iListTargetPos.next();
        if(pbTargetPos.targetid()%worldCount!=worldIndex)
            continue;

        Target *target=new Target(pbTargetPos,this,timeInt64);
        target->installPosDevices(timeInt64);
        qint32 targetID=pbTargetPos.targetid();
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
    }

    return true;
 }

 void ThreadedWorld::slotInitDataChannels(const QMap <PB_Enum_TargetInfo_Type, Struct_PosDeviceInfo> &mapInfoTypePosDeviceInfo)
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

