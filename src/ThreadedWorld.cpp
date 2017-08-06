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

ThreadedWorld::ThreadedWorld(QMutex *mutex, MyQtGeoPolygon *geoPolyGonBoundingRegion,PBCoderDecoder *pbCodDecoder,  QObject *parent) :
                            QObject(parent)
{
    this->sharedGeoPolyGonBoundingRegion=geoPolyGonBoundingRegion;

    colCount=GRID_ARRAY_ROW_COUNT*2;
    rowCount=GRID_ARRAY_ROW_COUNT;

    this->sharedMutex=mutex;

    this->sharedPbCoderDecoder=pbCodDecoder;

    parseParamFileAndInitMembers();
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

 void  ThreadedWorld::initTargetsAndAddToDataSources()
{
     QFile mc2File(mc2FileName);
     if (!mc2File.open(QIODevice::ReadOnly)) {
         qDebug()<<"Warning: Couldn't open "<<mc2File.fileName()<<mc2File.errorString()<<". Targets will not have country attributes.";
         return ;
     }

     QList <QString> listCountryNames;
     quint8 colInd=2;
     if(language.toLower()=="cn")
         colInd=1;

     mc2File.readLine(); //skip first line
     while (!mc2File.atEnd())
     {
         QList <QByteArray> listField = mc2File.readLine().trimmed().split(',');
         if(listField.size()<3)
         {
             qDebug()<<"Warning: Column count of  file"<<mc2File.fileName()<<" is not correct! This Line is:"<<listField;
             continue;
         }
         listCountryNames.append(listField.at(colInd));
     }

     QFile shipDataFile(ship_FileName);
     if (!shipDataFile.open(QIODevice::ReadOnly)) {
         qDebug()<<"Critical: Couldn't open "<<shipDataFile.fileName()<<shipDataFile.errorString()<<". Nothing will be done.";
         exit(3);
         return ;
     }

     qint16 countryCount=listCountryNames.size();
     qint32 targetID=0;
     shipDataFile.readLine(); //skip first line
     while (!shipDataFile.atEnd()&&targetID<(qint32)ExternV_TargetCount)
     {

         QList <QByteArray> listField = shipDataFile.readLine().trimmed().split(',');
#ifdef SHIP_DATA_ANONYMOUS
         if(listField.size()!=6)
         {
             qDebug()<<"Critical: Column count of ship file is not correct! Exiting....";
             exit(4);
             break;
         }
         qint32 longitudeX60W=listField.at(1).toInt();
         qint32 latitudeX60W=listField.at(2).toInt();
         qint32 cogX10=listField.at(3).toInt();
         qint32 sogX10=listField.at(4).toInt();
#endif

#ifndef SHIP_DATA_ANONYMOUS
         if(listField.size()!=18)
         {
             qDebug()<<"Critical: Column count of ship file is not correct! Column count is: "<<listField.size()<<"  Exiting...."<<listField;
             exit(4);
             break;
         }

         qint32 longitudeX60W=listField.at(1).toInt();
         qint32 latitudeX60W=listField.at(2).toInt();
         qint32 cogX10=listField.at(3).toInt();
         qint32 sogX10=listField.at(5).toInt();
         QString shipName=listField.at(8);
#endif

         if(sogX10<(qint32)ExternV_SOGX10_LOWER_THRESH || sogX10>(qint32)ExternV_SOGX10_UPPER_THRESH)
             continue;

         if(sharedGeoPolyGonBoundingRegion&&!sharedGeoPolyGonBoundingRegion->containsPoint(QGeoCoordinate(latitudeX60W/AISPosDivider,longitudeX60W/AISPosDivider)))
             continue;

         targetID++; //Start from 1

        PBTargetPosition pbTargetPosOrig;
        pbTargetPosOrig.set_targetid(targetID);
        pbTargetPosOrig.mutable_aisdynamic()->set_mmsi(EV_TargetIDType_MMSI*ExternV_TargetCount+targetID);
        pbTargetPosOrig.mutable_aisdynamic()->set_intlongitudex60w(longitudeX60W);
        pbTargetPosOrig.mutable_aisdynamic()->set_intlatitudex60w(latitudeX60W);
        pbTargetPosOrig.mutable_aisdynamic()->set_cogdegreex10(cogX10);
        pbTargetPosOrig.mutable_aisdynamic()->set_headingdegree(qRound(cogX10/10.0));
        pbTargetPosOrig.mutable_aisdynamic()->set_sogknotsx10(sogX10);
        pbTargetPosOrig.mutable_aisdynamic()->set_utctimestamp(QDateTime::currentDateTime().toTime_t());
        pbTargetPosOrig.set_enum_targetinfotype(EV_TargetInfoType_AISDynamic);
        pbTargetPosOrig.set_enum_targetidorig_type(EV_TargetIDType_MMSI);
        pbTargetPosOrig.set_targetidorig(EV_TargetIDType_MMSI*ExternV_TargetCount+targetID);

        pbTargetPosOrig.set_countryname(listCountryNames.at(qrand()%countryCount).toUtf8().toStdString());

        pbTargetPosOrig.mutable_aisstatic()->set_mmsi(EV_TargetIDType_MMSI*ExternV_TargetCount+targetID);
#ifndef SHIP_DATA_ANONYMOUS
        pbTargetPosOrig.mutable_aisstatic()->set_shipname(shipName.toStdString());
#endif
        pbTargetPosOrig.mutable_aisstatic()->set_shiptype_ais(qrand()%71+20); //20-90
        pbTargetPosOrig.mutable_aisstatic()->set_imo(EV_TargetIDType_IMO*ExternV_TargetCount+targetID);
        pbTargetPosOrig.set_aggregatedaisshiptype(PBCoderDecoder::getAggregatedAISShipType(pbTargetPosOrig.aisstatic().shiptype_ais()));

        pbTargetPosOrig.set_beidouid(EV_TargetIDType_BeidouID*ExternV_TargetCount+targetID);
        pbTargetPosOrig.set_haijianid(EV_TargetIDType_HaijianID*ExternV_TargetCount+targetID);
        pbTargetPosOrig.set_argosandmarinesatid(EV_TargetIDType_ArgosAndMarineSatID*ExternV_TargetCount+targetID);

        createOneTarget(targetID, pbTargetPosOrig,QDateTime::currentDateTime());
     }
     qDebug()<<"Number of targets created: "<<targetID;
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

 void ThreadedWorld::initDataChannels()
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

 QMap<PB_Enum_TargetInfo_Type, QSet<qint32> > &ThreadedWorld::getRefMapInfoTypeSetTargetID()
 {
     return mapInfoTypeSetTargetID;
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

