#include "World.h"
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
#include "macro.h"
#include "target.h"

World::World(QMutex *mutex,  QObject *parent) :
                            QObject(parent)
{
    //qsrand(QDateTime::currentDateTime().toTime_t());
    qsrand(0); //Make sure that every time the program started, the simulated scence is the same.

    colCount=GRID_ARRAY_ROW_COUNT*2;
    rowCount=GRID_ARRAY_ROW_COUNT;

    this->mutex=mutex;

    //pbCoderDecoderForAggregatedPBToSend=new PBCoderDecoder(SOFTWARE_NAME,this);
    pbCoderDecoder=new PBCoderDecoder(SOFTWARE_NAME,this);

    parseParamFileAndInitMembers();
    timerMeasureAndUpdateTargetPos=new QTimer(this);
    connect(timerMeasureAndUpdateTargetPos,&QTimer::timeout,this, &World::slotTimerEventMeasureAndUpdateTargetsPos);
    timerMeasureAndUpdateTargetPos->start(ExternV_Milliseconds_FetchData);

    timerOutputTargetCountAndMsgRate=new QTimer(this);
    connect(timerOutputTargetCountAndMsgRate, &QTimer::timeout,this,&World::slotTimerEventOutPutTargetCountAndMsgRate);
    timerOutputTargetCountAndMsgRate->start(ExternV_SECONDS_CHECK_TARGET_COUNT*1000);
}

void World::parseParamFileAndInitMembers()
{
    QFile paramJsonFile(QStringLiteral("param.json"));
    if (!paramJsonFile.open(QIODevice::ReadOnly)) {
        qDebug()<<"Critical: Couldn't open "<<paramJsonFile.fileName()<<paramJsonFile.errorString()<<". Nothing will be done.";
        exit(1);
        return ;
    }
    QByteArray jsonData = paramJsonFile.readAll();
    QJsonDocument jsonDoc( QJsonDocument::fromJson(jsonData));
    QJsonObject jsonObjcet=jsonDoc.object();

    if(checkJsonObjectAndOutPutValue(jsonObjcet,"ISDebugMode",true))
        ExternV_IS_DEBUG_MODE=jsonObjcet.value("ISDebugMode").toBool(ExternV_IS_DEBUG_MODE);
    if(checkJsonObjectAndOutPutValue(jsonObjcet,"TargetCount",true))
    {
        ExternV_TargetCount=jsonObjcet.value("TargetCount").toInt(ExternV_TargetCount);
    }
    if(checkJsonObjectAndOutPutValue(jsonObjcet,"SOGX10_LOWER_THRESH",true))
        SOGX10_LOWER_THRESH=jsonObjcet.value("SOGX10_LOWER_THRESH").toInt(SOGX10_LOWER_THRESH);
    if(checkJsonObjectAndOutPutValue(jsonObjcet,"SECONDS_CHECK_TARGET_COUNT",true))
        ExternV_SECONDS_CHECK_TARGET_COUNT=jsonObjcet.value("SECONDS_CHECK_TARGET_COUNT").toInt(ExternV_SECONDS_CHECK_TARGET_COUNT);
    if(checkJsonObjectAndOutPutValue(jsonObjcet,"WaterGridsFileName",true))
        waterGridsFileName=jsonObjcet.value("WaterGridsFileName").toString();
    if(checkJsonObjectAndOutPutValue(jsonObjcet,"Ship_FileName",true))
        ship_FileName=jsonObjcet.value("Ship_FileName").toString();
    if(checkJsonObjectAndOutPutValue(jsonObjcet,"MC2_FileName",true))
        mc2FileName=jsonObjcet.value("MC2_FileName").toString();
    if(checkJsonObjectAndOutPutValue(jsonObjcet,"Language",true))
        language=jsonObjcet.value("Language").toString();

    if(checkJsonObjectAndOutPutValue(jsonObjcet,"PosDevice",false))
    {
        QJsonArray arrayDevices=jsonObjcet.value("PosDevice").toArray();
        for(QJsonArray::const_iterator iArray=arrayDevices.constBegin(); iArray!=arrayDevices.constEnd();iArray++)
        {
            QJsonObject jsonObjInArray= (*iArray).toObject();
            if(checkJsonObjectAndOutPutValue(jsonObjInArray,"TargetInfo_Type",false)&&
                    checkJsonObjectAndOutPutValue(jsonObjInArray,"PositioningDevInMeters",false)&&
                    checkJsonObjectAndOutPutValue(jsonObjInArray,"SampleMilliSeconds",false))
            {
                PB_Enum_TargetInfo_Type infoType=(PB_Enum_TargetInfo_Type)jsonObjInArray.value("TargetInfo_Type").toInt(0);
                Struct_PosDeviceInfo deviceInfo;
                deviceInfo.infoType=infoType;
                deviceInfo.positioningDevInMeters=jsonObjInArray.value("PositioningDevInMeters").toDouble(0);
                deviceInfo.sampleMilliSeconds=jsonObjInArray.value("SampleMilliSeconds").toInt(10000);
                mapInfoTypePosDeviceInfo.insert(infoType, deviceInfo);
                qDebug()<<PBCoderDecoder::getReadableTargetInfo_TypeName(infoType)<<" { "<<"定位误差:"<<deviceInfo.positioningDevInMeters<<
                          "米\t采样间隔:"<<deviceInfo.sampleMilliSeconds/1000.0<<"秒"<<"}";
            }
            else
                qDebug()<<"Fail to parse jsonObject:"<<jsonObjInArray;
        }
    }

    initDataChannels();

    /****************************** Start  the iteration  of all Data sources********************************/
    if(checkJsonObjectAndOutPutValue(jsonObjcet,"DataSources",false))
    {
        QJsonArray arrayDataSources=jsonObjcet.value("DataSources").toArray();

        /****************************** iterate  Data source one by one***************************************/
        for(QJsonArray::const_iterator iArray=arrayDataSources.constBegin(); iArray!=arrayDataSources.constEnd();iArray++)
        {
            QJsonObject jsonDataSourceObjInArray= (*iArray).toObject(); //One data source
            if(checkJsonObjectAndOutPutValue(jsonDataSourceObjInArray,"TargetInfo_Source",false)&&
                    checkJsonObjectAndOutPutValue(jsonDataSourceObjInArray,"SourceInfo",false))
            {
                qint32 dataSourceID=jsonDataSourceObjInArray.value("TargetInfo_Source").toInt(0);
                QJsonArray arraySourceInfo=jsonDataSourceObjInArray.value("SourceInfo").toArray();
                QMap <PB_Enum_TargetInfo_Type,Struct_TransmissionQuality>  mapInfoTypeTransmitQualityOfOneDataSource;

                /****************************** iterate  sourceInfo of one Data source one by one*******************************/
                for(QJsonArray::const_iterator iArraySourceInfo=arraySourceInfo.constBegin(); iArraySourceInfo!=arraySourceInfo.constEnd();iArraySourceInfo++)
                {
                    QJsonObject jsonSourceInfoObjInArray= (*iArraySourceInfo).toObject(); //One  source info
                    if(     checkJsonObjectAndOutPutValue(jsonSourceInfoObjInArray,"TargetInfo_Type",false)&&
                            checkJsonObjectAndOutPutValue(jsonSourceInfoObjInArray,"ObservePercentage",false)&&
                            checkJsonObjectAndOutPutValue(jsonSourceInfoObjInArray,"meanTransmissionLatencyInMiliSeconds",false)&&
                            checkJsonObjectAndOutPutValue(jsonSourceInfoObjInArray,"stdDevTransmissionLatencyInMiliSeconds",false)&&
                            checkJsonObjectAndOutPutValue(jsonSourceInfoObjInArray,"meanTimestampErrorInMiliSeconds",false)&&
                            checkJsonObjectAndOutPutValue(jsonSourceInfoObjInArray,"stdDevTimestampErrorInMiliSeconds",false)&&
                            checkJsonObjectAndOutPutValue(jsonSourceInfoObjInArray,"packetLossPercentage",false))
                    {
                        PB_Enum_TargetInfo_Type infoType=(PB_Enum_TargetInfo_Type)jsonSourceInfoObjInArray.value("TargetInfo_Type").toInt(0);
                        if(!mapInfoTypePosDeviceInfo.contains(infoType))
                        {
                            qDebug()<<"This infoType is not configured in PosDevice of param.json, ignored: "<<
                                      PBCoderDecoder::getReadableTargetInfo_TypeName(infoType);
                            continue;
                        }
                        Struct_TransmissionQuality transQuality;
                        transQuality.infoType=infoType;
                        transQuality.percentageTargetsObserved=jsonSourceInfoObjInArray.value("ObservePercentage").toInt(100);
                        transQuality.meanTransmissionLatencyInMiliSeconds=jsonSourceInfoObjInArray.value("meanTransmissionLatencyInMiliSeconds").toInt(0);
                        transQuality.stdDevTransmissionLatencyInMiliSeconds=jsonSourceInfoObjInArray.value("stdDevTransmissionLatencyInMiliSeconds").toInt(0);
                        transQuality.meanTimestampErrorInMiliSeconds=jsonSourceInfoObjInArray.value("meanTimestampErrorInMiliSeconds").toInt(0);
                        transQuality.stdDevTimestampErrorInMiliSeconds=jsonSourceInfoObjInArray.value("stdDevTimestampErrorInMiliSeconds").toInt(0);
                        transQuality.packetLossPercentage=jsonSourceInfoObjInArray.value("packetLossPercentage").toInt(0);
                        mapInfoTypeTransmitQualityOfOneDataSource.insert(infoType, transQuality);

                        qDebug()<<PBCoderDecoder::getReadableTargetInfo_SourceName( (PB_Enum_TargetInfo_Source)dataSourceID)<<"{"<<
                                  "数据类型:"<<PBCoderDecoder::getReadableTargetInfo_TypeName(infoType)<<
                                  "观测到的目标百分比:"<<transQuality.percentageTargetsObserved<<
                                  "平均传输延迟(毫秒):"<<transQuality.meanTransmissionLatencyInMiliSeconds<<
                                  "传输延迟标准差(毫秒):"<<transQuality.stdDevTransmissionLatencyInMiliSeconds<<
                                  "平均时间戳误差(毫秒):"<<transQuality.meanTimestampErrorInMiliSeconds<<
                                  "时间戳误差的标准差(毫秒):"<<transQuality.stdDevTimestampErrorInMiliSeconds<<
                                  "丢包率(百分比):"<<transQuality.packetLossPercentage<<"}";
                    }
                    else
                        qDebug()<<"Fail to parse jsonObject of SourceInfo in One Data Source.";
                }
                /****************************** end the iteration  of all sourceInfos of one Data source***************/
               if(!mapInfoSourceDataSources.contains((PB_Enum_TargetInfo_Source)dataSourceID))
               {
                   DataSource *dataSource=new DataSource(this,(PB_Enum_TargetInfo_Source)dataSourceID, mapInfoTypeTransmitQualityOfOneDataSource,this);
                    connect(dataSource, &DataSource::sigSend2MQ,this,&World::sigSend2MQ);
                    mapInfoSourceDataSources.insert( (PB_Enum_TargetInfo_Source)dataSourceID,dataSource);
               }
            }
            else //
                qDebug()<<"Fail to parse jsonObject of One Data Source:"<<jsonDataSourceObjInArray;

            /****************************** end of the iteration  of one Data source********************************/
        }
        /****************************** end  the iteration  of ALL Data sources****************************************/
    }
    else
        qDebug()<<"Fail to parse jsonObject of Data Sources:"<<jsonObjcet; //No data sources in the json

    initTargetsAndAddToDataSources();
    initWaterGrids();

}

World::~World()
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

void World::slotTimerEventMeasureAndUpdateTargetsPos()
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

void World::slotTimerEventOutPutTargetCountAndMsgRate()
{
    qint32 targetCountSum=0;
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
    }
    std::cout<< QDateTime::currentDateTime().toString("MM/dd hh:mm:ss").toStdString()<<"\t各数据源消息率总计:"<<
                QString::number(msgCountPerMinuteCount,'g',3).toStdString()<<"/分钟\t发送的总轨迹点数:"<<messageCountSum;

    QSet <qint32> setDistinctTargetIDOrig;
    QMapIterator <PB_Enum_TargetInfo_Type,QSet <qint32>> iMapInfoTypeSetTargetID(mapInfoTypeSetTargetID);
    while(iMapInfoTypeSetTargetID.hasNext())
    {
        iMapInfoTypeSetTargetID.next();
        qint32 setSize=iMapInfoTypeSetTargetID.value().size();
        setDistinctTargetIDOrig.unite(iMapInfoTypeSetTargetID.value());
        //qDebug()<<PBCoderDecoder::getReadableTargetInfo_TypeName(iMapInfoTypeSetTargetID.key())<<iMapInfoTypeSetTargetID.value();
        targetCountSum+=setSize;
        std::cout<<"\t"<<PBCoderDecoder::getReadableTargetInfo_TypeName(iMapInfoTypeSetTargetID.key()).toStdString()<<"目标总数:"<<setSize;
    }

    std::cout<<"\t各数据源不同原始编号目标总数(去重):"<<setDistinctTargetIDOrig.size()<<"\t各数据源目标数的和(不去重):"<<targetCountAll<<endl;

#ifdef DEBUG_TargetCount
    qDebug()<<"mapInfoTypeOrigTargetIDForDebug size is:"<<multiMapInfoTypeOrigTargetIDForDebug.size()<<". Contents are:"<<multiMapInfoTypeOrigTargetIDForDebug;
#endif

#ifdef DEBUG_PERFORMANCE
    qDebug()<<"Milliseconds to output target count:"<<  time.elapsed();
    time.start();
#endif
}

 void  World::initTargetsAndAddToDataSources()
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

     QFile paramJsonFile(ship_FileName);
     if (!paramJsonFile.open(QIODevice::ReadOnly)) {
         qDebug()<<"Critical: Couldn't open "<<paramJsonFile.fileName()<<paramJsonFile.errorString()<<". Nothing will be done.";
         exit(3);
         return ;
     }

     qint16 countryCount=listCountryNames.size();
     qint32 targetID=0;
     while (!paramJsonFile.atEnd()&&targetID<(qint32)ExternV_TargetCount)
     {

         QList <QByteArray> listField = paramJsonFile.readLine().trimmed().split(',');
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

         if(sogX10<(qint32)SOGX10_LOWER_THRESH)
             continue;

         targetID++; //Start from 1

        PBTargetPosition pbTargetPosOrig;
        pbTargetPosOrig.set_targetid(targetID);
        pbTargetPosOrig.mutable_aisdynamic()->set_mmsi(EV_TargetIDType_MMSI*ExternV_TargetCount+targetID);
        pbTargetPosOrig.mutable_aisdynamic()->set_intlongitudex60w(longitudeX60W);
        pbTargetPosOrig.mutable_aisdynamic()->set_intlatitudex60w(latitudeX60W);
        pbTargetPosOrig.mutable_aisdynamic()->set_cogdegreex10(cogX10);
        pbTargetPosOrig.mutable_aisdynamic()->set_sogknotsx10(sogX10);
        pbTargetPosOrig.mutable_aisdynamic()->set_utctimestamp(QDateTime::currentDateTime().toTime_t());
        pbTargetPosOrig.set_enum_targetinfotype(EV_TargetInfoType_AISDynamic);
        pbTargetPosOrig.set_enum_targetidorig_type(EV_TargetIDType_MMSI);
        pbTargetPosOrig.set_targetidorig(EV_TargetIDType_MMSI*ExternV_TargetCount+targetID);

        pbTargetPosOrig.set_countryname(listCountryNames.at(qrand()%countryCount).toUtf8().toStdString());

        pbTargetPosOrig.mutable_aisstatic()->set_mmsi(EV_TargetIDType_MMSI*ExternV_TargetCount+targetID);
        pbTargetPosOrig.mutable_aisstatic()->set_shiptype_ais(qrand()%100);
        pbTargetPosOrig.mutable_aisstatic()->set_imo(EV_TargetIDType_IMO*ExternV_TargetCount+targetID);
        pbTargetPosOrig.set_aggregatedaisshiptype(PBCoderDecoder::getAggregatedAISShipType(pbTargetPosOrig.aisstatic().shiptype_ais()));

        pbTargetPosOrig.set_beidouid(EV_TargetIDType_BeidouID*ExternV_TargetCount+targetID);
        pbTargetPosOrig.set_haijianid(EV_TargetIDType_HaijianID*ExternV_TargetCount+targetID);
        pbTargetPosOrig.set_argosandmarinesatid(EV_TargetIDType_ArgosAndMarineSatID*ExternV_TargetCount+targetID);

        Target *target=new Target(pbTargetPosOrig,this,QDateTime::currentDateTime());
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
     }
     qDebug()<<"Number of targets created: "<<targetID;
 }

 void World::initDataChannels()
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


 bool World::checkJsonObjectAndOutPutValue(const QJsonObject &jsonObject,  const QString &key, const bool &outPutValue)
{
     if(jsonObject.contains(key))
     {
         if(outPutValue&& !jsonObject.value(key).isObject()&&!jsonObject.value(key).isArray())
         {
             //qDebug()<<endl<<"---------------Checking JsonObject key----------------------";
            qDebug()<<key<<":"<<jsonObject.value(key);
         }
         return true;
     }
     else
         return false;
 }

 
 PBCoderDecoder *World::getPbCoderDecoder() const
 {
     return pbCoderDecoder;
 }
 
 QMap<PB_Enum_TargetInfo_Type, DataChannel *> World::getMapInfoTypeDataChannels() const
 {
     return mapInfoTypeDataChannels;
 }
 
 QMap<PB_Enum_TargetInfo_Type, Struct_PosDeviceInfo> World::getMapInfoTypePosDeviceInfo() const
 {
     return mapInfoTypePosDeviceInfo;
 }

 QHash<qint32, Target *> World::getHashIDTarget() const
 {
     return hashIDTarget;
 }

