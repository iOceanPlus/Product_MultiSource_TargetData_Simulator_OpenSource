#include <QFile>
#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include "simulator.h"
#include "MyQtGeoPolygon.h"
#include "PBCoderDecoder.h"

Simulator::Simulator(QObject *parent) : QObject(parent)
{
    this->thread()->setPriority(QThread::LowPriority);
    mutex=new QMutex();
    sharedGeoPolyGonBoundingRegion=NULL;
    qsrand(0); //Make sure that every time the program started, the simulated scence is the same.

    worldThreadCount=1; //default count
    setWorldThreadCountAndBoundingRegionFromParamJson();

    pbCoderDecoder=new PBCoderDecoder(SOFTWARE_NAME,mutex,this);
    for(int i=0;i<worldThreadCount;i++)
    {
        QThread *thread=new QThread(this);
        listOfWorldThreads.append(thread);
        ThreadedWorld *world=new ThreadedWorld(mutex,sharedGeoPolyGonBoundingRegion,pbCoderDecoder,i);
        listOfThreadedWorlds.append(world);
        connect(thread,&QThread::finished,world,&QObject::deleteLater);
        world->moveToThread(thread);
        thread->start(QThread::NormalPriority);
    }
    QStringList listRoutingKeyToConsume;
    listRoutingKeyToConsume.append(ROUTING_KEY_MONITOR_RPOBE);
    ioMessages=new IOMessages(SOFTWARE_NAME,listRoutingKeyToConsume,"param_mq.txt",this);
    connectIOMessageAndWorld();

    parseParamFileAndInitWorldMembers();
    timerOutputTargetCountAndMsgRate=new QTimer(this);
    connect(timerOutputTargetCountAndMsgRate, &QTimer::timeout,this,&Simulator::slotTimerEventOutPutTargetCountAndMsgRate);
    timerOutputTargetCountAndMsgRate->start(ExternV_SECONDS_CHECK_TARGET_COUNT*1000);
}


void Simulator::connectIOMessageAndWorld()
{
    QListIterator <ThreadedWorld*> iListWorlds(listOfThreadedWorlds);
    while(iListWorlds.hasNext())
    {
        ThreadedWorld *world= iListWorlds.next();
        connect(ioMessages,SIGNAL(sigPBMonitor(PBMonitor)),world,SIGNAL(sigPBMonitor(PBMonitor)));

        connect(world,SIGNAL(sigSend2MQ(QList<StructDataAndKey>)),
                ioMessages,SLOT(slotPublishToMQ(QList<StructDataAndKey>)));

        connect(this,&Simulator::sigAddDataSourceIfNotExist,world,&ThreadedWorld::slotAddDataSourceIfNotExist);
        connect(this,SIGNAL(sigCreateTargets(QList<PBTargetPosition>,quint16)),world,SLOT(slotCreateTargets(QList<PBTargetPosition>,quint16)));
        connect(this,&Simulator::sigInitDataChannels,world, &ThreadedWorld::slotInitDataChannels);
    }
}

bool Simulator::setWorldThreadCountAndBoundingRegionFromParamJson()
{
    QFile paramJsonFile(QStringLiteral("param.json"));
    if (!paramJsonFile.open(QIODevice::ReadOnly)) {
        qDebug()<<"Critical: Couldn't open "<<paramJsonFile.fileName()<<paramJsonFile.errorString()<<". Nothing will be done.";
        exit(1);
        return false;
    }
    QByteArray jsonData = paramJsonFile.readAll();
    QJsonDocument jsonDoc( QJsonDocument::fromJson(jsonData));
    QJsonObject jsonObjcet=jsonDoc.object();
    if(checkJsonObjectAndOutPutValue(jsonObjcet,"Bounding_Region",true))
    {
        QVector<QPointF> vectPoints;
        QStringList listStrPoings=jsonObjcet.value("Bounding_Region").toString().simplified().split(",");
        if(listStrPoings.size()>=4)
        {
            QStringListIterator iStrPoints(listStrPoings);
            while(iStrPoints.hasNext())
            {
                QStringList strPoint= iStrPoints.next().simplified().split(" ");
                if(strPoint.size()!=2)
                {
                    qDebug()<<"ERROR: Fail to parse a point of Bounding_Region."<<strPoint<<" Exiting...";
                    exit(6);
                    return false;
                }
                else
                {
                    QPointF pf(strPoint.at(1).toFloat(), strPoint.at(0).toFloat());
                    vectPoints.append(pf);
                }
            }
            bool ok;
            sharedGeoPolyGonBoundingRegion=new MyQtGeoPolygon(vectPoints,&ok,1,"bounding",this);
        }
    }

    if(checkJsonObjectAndOutPutValue(jsonObjcet,"World_Threads_Count",true))
    {
        worldThreadCount=jsonObjcet.value("World_Threads_Count").toInt(worldThreadCount);
        return true;
    }
    return false;
}

void Simulator::parseParamFileAndInitWorldMembers()
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
    if(checkJsonObjectAndOutPutValue(jsonObjcet,"World_Threads_Count",true))
    {
        worldThreadCount=jsonObjcet.value("World_Threads_Count").toInt(worldThreadCount);
    }
    if(checkJsonObjectAndOutPutValue(jsonObjcet,"ExternV_MIN_Sample_MSEC",true))
    {
        ExternV_MIN_Sample_MSEC=jsonObjcet.value("ExternV_MIN_Sample_MSEC").toInt(ExternV_MIN_Sample_MSEC);
    }
    if(checkJsonObjectAndOutPutValue(jsonObjcet,"ExternV_Milliseconds_FetchData",true))
    {
        ExternV_Milliseconds_FetchData=jsonObjcet.value("ExternV_Milliseconds_FetchData").toInt(ExternV_Milliseconds_FetchData);
    }

    if(checkJsonObjectAndOutPutValue(jsonObjcet,"SOGX10_LOWER_THRESH",true))
        ExternV_SOGX10_LOWER_THRESH=jsonObjcet.value("SOGX10_LOWER_THRESH").toInt(ExternV_SOGX10_LOWER_THRESH);
    if(checkJsonObjectAndOutPutValue(jsonObjcet,"SOGX10_UPPER_THRESH",true))
        ExternV_SOGX10_UPPER_THRESH=jsonObjcet.value("SOGX10_UPPER_THRESH").toInt(ExternV_SOGX10_UPPER_THRESH);

//    if(checkJsonObjectAndOutPutValue(jsonObjcet,"LATITUDE_LOWER_THRESH_DEGREE",true))
//        ExternV_LATITUDE_LOWER_THRESH_DEGREE=jsonObjcet.value("LATITUDE_LOWER_THRESH_DEGREE").toDouble(ExternV_LATITUDE_LOWER_THRESH_DEGREE);

//    if(checkJsonObjectAndOutPutValue(jsonObjcet,"LATITUDE_UPPER_THRESH_DEGREE",true))
//        ExternV_LATITUDE_UPPER_THRESH_DEGREE=jsonObjcet.value("LATITUDE_UPPER_THRESH_DEGREE").toDouble(ExternV_LATITUDE_UPPER_THRESH_DEGREE);

//    if(checkJsonObjectAndOutPutValue(jsonObjcet,"LONGITUDE_LOWER_THRESH_DEGREE",true))
//        ExternV_LONGITUDE_LOWER_THRESH_DEGREE=jsonObjcet.value("LONGITUDE_LOWER_THRESH_DEGREE").toDouble(ExternV_LONGITUDE_LOWER_THRESH_DEGREE);

//    if(checkJsonObjectAndOutPutValue(jsonObjcet,"LONGITUDE_UPPER_THRESH_DEGREE",true))
//        ExternV_LONGITUDE_UPPER_THRESH_DEGREE=jsonObjcet.value("LONGITUDE_UPPER_THRESH_DEGREE").toDouble(ExternV_LONGITUDE_UPPER_THRESH_DEGREE);

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

    emit sigInitDataChannels(mapInfoTypePosDeviceInfo);


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
              emit sigAddDataSourceIfNotExist((PB_Enum_TargetInfo_Source)dataSourceID,mapInfoTypeTransmitQualityOfOneDataSource);
            }
            else //
                qDebug()<<"Fail to parse jsonObject of One Data Source:"<<jsonDataSourceObjInArray;

            /****************************** end of the iteration  of one Data source********************************/
        }
        /****************************** end  the iteration  of ALL Data sources****************************************/
    }
    else
        qDebug()<<"Fail to parse jsonObject of Data Sources:"<<jsonObjcet; //No data sources in the json

    initTargetsAndPutToWorlds();
    initWaterGrids();
}

void  Simulator::initTargetsAndPutToWorlds()
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
    QList<PBTargetPosition> listPbTargetPos;
    qint16 countryCount=listCountryNames.size();
    qint32 targetID=0;
    shipDataFile.readLine(); //skip first line
    quint32 timeInt32= QDateTime::currentDateTime().toTime_t();
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

       PBTargetPosition pbTargetPos;
       pbTargetPos.set_targetid(targetID);
       pbTargetPos.mutable_aisdynamic()->set_mmsi(EV_TargetIDType_MMSI*ExternV_TargetCount+targetID);
       pbTargetPos.mutable_aisdynamic()->set_intlongitudex60w(longitudeX60W);
       pbTargetPos.mutable_aisdynamic()->set_intlatitudex60w(latitudeX60W);
       pbTargetPos.mutable_aisdynamic()->set_cogdegreex10(cogX10);
       pbTargetPos.mutable_aisdynamic()->set_headingdegree(qRound(cogX10/10.0));
       pbTargetPos.mutable_aisdynamic()->set_sogknotsx10(sogX10);
       pbTargetPos.mutable_aisdynamic()->set_utctimestamp(timeInt32);
       pbTargetPos.set_enum_targetinfotype(EV_TargetInfoType_AISDynamic);
       pbTargetPos.set_enum_targetidorig_type(EV_TargetIDType_MMSI);
       pbTargetPos.set_targetidorig(EV_TargetIDType_MMSI*ExternV_TargetCount+targetID);

       pbTargetPos.set_countryname(listCountryNames.at(qrand()%countryCount).toUtf8().toStdString());

       pbTargetPos.mutable_aisstatic()->set_mmsi(EV_TargetIDType_MMSI*ExternV_TargetCount+targetID);
#ifndef SHIP_DATA_ANONYMOUS
       pbTargetPos.mutable_aisstatic()->set_shipname(shipName.toStdString());
#endif
       pbTargetPos.mutable_aisstatic()->set_shiptype_ais(qrand()%71+20); //20-90
       pbTargetPos.mutable_aisstatic()->set_imo(EV_TargetIDType_IMO*ExternV_TargetCount+targetID);
       pbTargetPos.set_aggregatedaisshiptype(PBCoderDecoder::getAggregatedAISShipType(pbTargetPos.aisstatic().shiptype_ais()));

       pbTargetPos.set_beidouid(EV_TargetIDType_BeidouID*ExternV_TargetCount+targetID);
       pbTargetPos.set_haijianid(EV_TargetIDType_HaijianID*ExternV_TargetCount+targetID);
       pbTargetPos.set_argosandmarinesatid(EV_TargetIDType_ArgosAndMarineSatID*ExternV_TargetCount+targetID);

       listPbTargetPos.append(pbTargetPos);
    }
    emit sigCreateTargets(listPbTargetPos, listOfWorldThreads.size());
    qDebug()<<"Number of targets created: "<<targetID;
}


void Simulator::initWaterGrids()
{
    for(int rInd=0; rInd<(qint32)GRID_ARRAY_ROW_COUNT;rInd++)
    {
        for(int cInd=0;cInd<(qint32)(2*GRID_ARRAY_ROW_COUNT);cInd++)
        {
            externVIsWater[rInd][cInd]=false;
        }
    }

    QFile paramJsonFile(waterGridsFileName);
    if (!paramJsonFile.open(QIODevice::ReadOnly)) {
        qDebug()<<"Critical: Couldn't open "<<paramJsonFile.fileName()<<paramJsonFile.errorString()<<". Nothing will be done.";
        exit(1);
        return ;
    }

    while (!paramJsonFile.atEnd())
    {
        QList <QByteArray> listField = paramJsonFile.readLine().trimmed().split(',');
        if(listField.size()!=4)
        {
            qDebug()<<"Critical: Column count of water Grid file is not correct!";
            exit(2);
            break;
        }
        qint32 rowInd,colInd;
        ThreadedWorld::getGridIndex( (listField.at(0).toInt()+listField.at(2).toInt())/AISPosDivider/2.0, (listField.at(1).toInt()+listField.at(3).toInt())/AISPosDivider/2.0,rowInd,colInd);
        if(rowInd>=0&&rowInd<(qint32)GRID_ARRAY_ROW_COUNT&&
                colInd>=0&&colInd<(qint32)GRID_ARRAY_ROW_COUNT*2)
               externVIsWater[rowInd][colInd]=true;
    }
}

bool Simulator::checkJsonObjectAndOutPutValue(const QJsonObject &jsonObject,  const QString &key, const bool &outPutValue)
{
    if(jsonObject.contains(key))
    {
        if(outPutValue&& !jsonObject.value(key).isObject()&&!jsonObject.value(key).isArray())
        {
           qDebug()<<key<<":"<<jsonObject.value(key);
        }
        return true;
    }
    else
        return false;
}

void Simulator::slotTimerEventOutPutTargetCountAndMsgRate()
{
    qint32 targetCountSumAccordingToInfoTypeAndOrigTargetID=0;
    qint32 targetCountAll=0;
    quint64 messageCountSum=0;
    float msgCountPerMinuteCount=0;

#ifdef DEBUG_PERFORMANCE
    QTime time;
    time.start();
#endif

    QListIterator <ThreadedWorld*> iListWorlds(listOfThreadedWorlds);
    while(iListWorlds.hasNext())
    {
        iListWorlds.next()->updateTargetCountAndMsgRate(setDistinctTargetIDOrig,mapInfoTypeSetTargetID, targetCountAll, msgCountPerMinuteCount,messageCountSum);
    }

    std::cout<< QDateTime::currentDateTime().toString("MM/dd hh:mm:ss").toStdString()<<"\t各数据源消息率总计:"<<
                QString::number(msgCountPerMinuteCount,'g',3).toStdString()<<"/分钟\t发送的总轨迹点数:"<<messageCountSum<<endl;

    QMapIterator <PB_Enum_TargetInfo_Type,QSet <qint32>> iMapInfoTypeSetTargetID(mapInfoTypeSetTargetID);
    while(iMapInfoTypeSetTargetID.hasNext())
    {
        iMapInfoTypeSetTargetID.next();
        qint32 setSize=iMapInfoTypeSetTargetID.value().size();
        targetCountSumAccordingToInfoTypeAndOrigTargetID+=setSize;
        std::cout<<"\t"<<PBCoderDecoder::getReadableTargetInfo_TypeName(iMapInfoTypeSetTargetID.key()).toStdString()<<"目标总数:"<<setSize;
    }

    std::cout<<endl<<"\t各数据源不同原始编号目标总数(去重):"<<setDistinctTargetIDOrig.size()<<
               "\t<信息类型-目标原始ID>组合数量："<<targetCountSumAccordingToInfoTypeAndOrigTargetID<<
               "\t各数据源目标数的和(不去重):"<<targetCountAll<<endl;

#ifdef DEBUG_TargetCount
    qDebug()<<"mapInfoTypeOrigTargetIDForDebug size is:"<<multiMapInfoTypeOrigTargetIDForDebug.size()<<". Contents are:"<<multiMapInfoTypeOrigTargetIDForDebug;
#endif

#ifdef DEBUG_PERFORMANCE
//    qDebug()<<"Milliseconds to output target count:"<<  time.elapsed();
//    time.start();
#endif
}


Simulator::~Simulator()
{
    QListIterator <QThread*> iListThreads(listOfWorldThreads);
    while(iListThreads.hasNext())
    {
        QThread *thread=iListThreads.next();
        if(thread&&thread->isRunning())
        {
            thread->quit();
            thread->wait(500);
            thread->deleteLater();
        }
        else if(thread)
            thread->deleteLater();
    }

    // Optional:  Delete all global objects allocated by libprotobuf.
    google::protobuf::ShutdownProtobufLibrary();
}
