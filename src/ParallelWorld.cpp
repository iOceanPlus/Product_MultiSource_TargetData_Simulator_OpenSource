#include "ParallelWorld.h"
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
#include "macro.h"
#include "target.h"

ParallelWorld::ParallelWorld(QMutex *mutex,  QObject *parent) :
                            QObject(parent)
{
    qsrand(QDateTime::currentDateTime().toTime_t());

    colCount=GRID_ARRAY_ROW_COUNT*2;
    rowCount=GRID_ARRAY_ROW_COUNT;

    this->mutex=mutex;

    //pbCoderDecoderForAggregatedPBToSend=new PBCoderDecoder(SOFTWARE_NAME,this);
    pbCoderDecoder=new PBCoderDecoder(SOFTWARE_NAME,this);

    parseParamFileAndInitMembers();
}

void ParallelWorld::parseParamFileAndInitMembers()
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

    if(checkJsonObjectAndOutPutValue(jsonObjcet,"ISDebugMode"))
        ExternV_IS_DEBUG_MODE=jsonObjcet.value("ISDebugMode").toBool(ExternV_IS_DEBUG_MODE);
    if(checkJsonObjectAndOutPutValue(jsonObjcet,"TargetCount"))
        ExternV_TargetCount=jsonObjcet.value("TargetCount").toInt(ExternV_TargetCount);
    if(checkJsonObjectAndOutPutValue(jsonObjcet,"SECONDS_CHECK_TARGET_COUNT"))
        ExternV_SECONDS_CHECK_TARGET_COUNT=jsonObjcet.value("SECONDS_CHECK_TARGET_COUNT").toInt(ExternV_SECONDS_CHECK_TARGET_COUNT);
    if(checkJsonObjectAndOutPutValue(jsonObjcet,"WaterGridsFileName"))
        waterGridsFileName=jsonObjcet.value("WaterGridsFileName").toString();
    if(checkJsonObjectAndOutPutValue(jsonObjcet,"Ship_FileName"))
        ship_FileName=jsonObjcet.value("Ship_FileName").toString();

    if(checkJsonObjectAndOutPutValue(jsonObjcet,"PosDevice"))
    {
        QJsonArray arrayDevices=jsonObjcet.value("PosDevice").toArray();
        for(QJsonArray::const_iterator iArray=arrayDevices.constBegin(); iArray!=arrayDevices.constEnd();iArray++)
        {
            QJsonObject jsonObjInArray= (*iArray).toObject();
            if(checkJsonObjectAndOutPutValue(jsonObjInArray,"TargetInfo_Type")&&
                    checkJsonObjectAndOutPutValue(jsonObjInArray,"PositioningDevInMeters")&&
                    checkJsonObjectAndOutPutValue(jsonObjInArray,"SampleMilliSeconds"))
            {
                PB_Enum_TargetInfo_Type infoType=(PB_Enum_TargetInfo_Type)jsonObjInArray.value("TargetInfo_Type").toInt(0);
                Struct_PosDeviceInfo deviceInfo;
                deviceInfo.infoType=infoType;
                deviceInfo.positioningDevInMeters=jsonObjInArray.value("PositioningDevInMeters").toDouble(0);
                deviceInfo.sampleMilliSeconds=jsonObjInArray.value("SampleMilliSeconds").toInt(10000);
                mapInfoTypePosDeviceInfo.insert(infoType, deviceInfo);
            }
            else
                qDebug()<<"Fail to parse jsonObject:"<<jsonObjInArray;
        }
    }

    initDataChannels();

    /****************************** Start  the iteration  of all Data sources********************************/
    if(checkJsonObjectAndOutPutValue(jsonObjcet,"DataSources"))
    {
        QJsonArray arrayDataSources=jsonObjcet.value("DataSources").toArray();

        /****************************** iterate  Data source one by one***************************************/
        for(QJsonArray::const_iterator iArray=arrayDataSources.constBegin(); iArray!=arrayDataSources.constEnd();iArray++)
        {
            QJsonObject jsonDataSourceObjInArray= (*iArray).toObject(); //One data source
            if(checkJsonObjectAndOutPutValue(jsonDataSourceObjInArray,"TargetInfo_Source")&&
                    checkJsonObjectAndOutPutValue(jsonDataSourceObjInArray,"SourceInfo"))
            {
                qint32 dataSourceID=jsonDataSourceObjInArray.value("TargetInfo_Source").toInt(0);
                QJsonArray arraySourceInfo=jsonDataSourceObjInArray.value("SourceInfo").toArray();
                QMap <PB_Enum_TargetInfo_Type,Struct_TransmissionQuality>  mapInfoTypeTransmitQualityOfOneDataSource;

                /****************************** iterate  sourceInfo of one Data source one by one*******************************/
                for(QJsonArray::const_iterator iArraySourceInfo=arraySourceInfo.constBegin(); iArraySourceInfo!=arraySourceInfo.constEnd();iArraySourceInfo++)
                {
                    QJsonObject jsonSourceInfoObjInArray= (*iArraySourceInfo).toObject(); //One  source info
                    if(     checkJsonObjectAndOutPutValue(jsonDataSourceObjInArray,"TargetInfo_Type")&&
                            checkJsonObjectAndOutPutValue(jsonDataSourceObjInArray,"ObservePercentage")&&
                            checkJsonObjectAndOutPutValue(jsonDataSourceObjInArray,"meanTransmissionLatencyInMiliSeconds")&&
                            checkJsonObjectAndOutPutValue(jsonDataSourceObjInArray,"stdDevTransmissionLatencyInMiliSeconds")&&
                            checkJsonObjectAndOutPutValue(jsonDataSourceObjInArray,"meanTimestampErrorInMiliSeconds")&&
                            checkJsonObjectAndOutPutValue(jsonDataSourceObjInArray,"stdDevTimestampErrorInMiliSeconds")&&
                            checkJsonObjectAndOutPutValue(jsonDataSourceObjInArray,"packetLossPercentage"))
                    {
                        PB_Enum_TargetInfo_Type infoType=(PB_Enum_TargetInfo_Type)jsonDataSourceObjInArray.value("TargetInfo_Type").toInt(0);
                        if(!mapInfoTypePosDeviceInfo.contains(infoType))
                        {
                            qDebug()<<"This infoType is not configured in PosDevice of param.json, ignored: "<<infoType;
                            continue;
                        }
                        Struct_TransmissionQuality transQuality;
                        transQuality.infoType=infoType;
                        transQuality.percentageTargetsObserved=jsonDataSourceObjInArray.value("ObservePercentage").toInt(100);
                        transQuality.meanTransmissionLatencyInMiliSeconds=jsonDataSourceObjInArray.value("meanTransmissionLatencyInMiliSeconds").toInt(0);
                        transQuality.stdDevTransmissionLatencyInMiliSeconds=jsonDataSourceObjInArray.value("stdDevTransmissionLatencyInMiliSeconds").toInt(0);
                        transQuality.meanTimestampErrorInMiliSeconds=jsonDataSourceObjInArray.value("meanTimestampErrorInMiliSeconds").toInt(0);
                        transQuality.stdDevTimestampErrorInMiliSeconds=jsonDataSourceObjInArray.value("stdDevTimestampErrorInMiliSeconds").toInt(0);
                        transQuality.packetLossPercentage=jsonDataSourceObjInArray.value("packetLossPercentage").toInt(0);
                        mapInfoTypeTransmitQualityOfOneDataSource.insert(infoType, transQuality);
                    }
                    else
                        qDebug()<<"Fail to parse jsonObject of SourceInfo in One Data Source:"<<jsonSourceInfoObjInArray;
                }
                /****************************** end the iteration  of all sourceInfos of one Data source***************/
               if(!mapInfoSourceDataSources.contains((PB_Enum_TargetInfo_Source)dataSourceID))
                    mapInfoSourceDataSources.insert( (PB_Enum_TargetInfo_Source)dataSourceID,
                                                 new DataSource(this,mapInfoTypeTransmitQualityOfOneDataSource,this));
            }
            else //
                qDebug()<<"Fail to parse jsonObject of One Data Source:"<<jsonDataSourceObjInArray;

            /****************************** end of the iteration  of one Data source********************************/
        }
        /****************************** end  the iteration  of ALL Data sources****************************************/
    }
    else
        qDebug()<<"Fail to parse jsonObject of Data Sources:"<<jsonObjcet; //No data sources in the json

    initWaterGrids();
    initTargetsAndDataSources();
}

ParallelWorld::~ParallelWorld()
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

void ParallelWorld::slotTimerEventMeasureAndUpdateTargetsPos()
{









}

 void  ParallelWorld::initTargetsAndDataSources()
{





 }

 void ParallelWorld::initDataChannels()
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


 bool ParallelWorld::checkJsonObjectAndOutPutValue(const QJsonObject &jsonObject,  const QString &key)
{
     if(jsonObject.contains(key))
     {
         qDebug()<<key<<":"<<jsonObject.value(key);
         return true;
     }
     else
         return false;
}

 QMap<PB_Enum_TargetInfo_Type, Struct_PosDeviceInfo> ParallelWorld::getMapInfoTypePosDeviceInfo() const
 {
     return mapInfoTypePosDeviceInfo;
 }

 QHash<qint32, Target *> ParallelWorld::getHashIDTarget() const
 {
     return hashIDTarget;
 }

