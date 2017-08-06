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
    geoPolyGonBoundingRegion=NULL;
    qsrand(0); //Make sure that every time the program started, the simulated scence is the same.

    worldThreadCount=1; //default count
    setWorldThreadCountAndBoundingRegionFromParamJson();
    pbCoderDecoder=new PBCoderDecoder(SOFTWARE_NAME,mutex,this);
    for(int i=0;i<worldThreadCount;i++)
    {
        QThread *thread=new QThread(this);
        listOfWorldThreads.append(thread);
        ThreadedWorld *world=new ThreadedWorld(mutex,geoPolyGonBoundingRegion,pbCoderDecoder);
        listOfThreadedWorlds.append(world);
        connect(thread,&QThread::finished,world,&QObject::deleteLater);
        world->moveToThread(thread);
        thread->start(QThread::NormalPriority);
    }
    parseParamFileAndInitMembers();
    QStringList listRoutingKeyToConsume;
    listRoutingKeyToConsume.append(ROUTING_KEY_MONITOR_RPOBE);
    ioMessages=new IOMessages(SOFTWARE_NAME,listRoutingKeyToConsume,"param_mq.txt",this);
    connectIOMessageAndWorld();
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
            geoPolyGonBoundingRegion=new MyQtGeoPolygon(vectPoints,&ok,1,"bounding",this);
        }
    }

    if(checkJsonObjectAndOutPutValue(jsonObjcet,"World_Threads_Count",true))
    {
        worldThreadCount=jsonObjcet.value("World_Threads_Count").toInt(worldThreadCount);
        return true;
    }
    return false;
}

void Simulator::parseParamFileAndInitMembers()
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
                    connect(dataSource, &DataSource::sigSend2MQ,this,&ThreadedWorld::sigSend2MQ);
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

Simulator::~Simulator()
{
    if(listOfWorldThreads&&listOfWorldThreads->isRunning())
    {
        listOfWorldThreads->quit();
        listOfWorldThreads->wait(500);
        listOfWorldThreads->deleteLater();
    }
    else if(listOfWorldThreads)
        listOfWorldThreads->deleteLater();

    // Optional:  Delete all global objects allocated by libprotobuf.
    google::protobuf::ShutdownProtobufLibrary();
}
