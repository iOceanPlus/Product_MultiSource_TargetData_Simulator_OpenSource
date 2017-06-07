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
using namespace std;

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

    QMapIterator <PB_Enum_TargetInfo_Type, Struct_PosDeviceInfo> iMapInfoTypePosDeviceInfo(mapInfoTypePosDeviceInfo);
    while(iMapInfoTypePosDeviceInfo.hasNext())
    {
        iMapInfoTypePosDeviceInfo.next();
        DataChannel *dataChannel=new DataChannel(this,iMapInfoTypePosDeviceInfo.key(),this);
        mapInfoTypeDataChannels.insert(iMapInfoTypePosDeviceInfo.key(),dataChannel);
    }

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
    initTargets();
    initDataChannels();
    initDataSources();
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

 void  ParallelWorld::initTargets()
{





 }

 void ParallelWorld::initDataSources()
{






 }

 void ParallelWorld::initDataChannels()
 {

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


 bool ParallelWorld::getLocation(quint32 rowIndex, quint32 colIndex,
                          double &lowerLeftLongitudeInDegree, double &lowerLeftLatidueInDegree)
 {
     if(rowIndex>=rowCount||colIndex>=colCount) //超出范围
     {
         qDebug()<<"rowIndex or colIndex is out of range in getLocation()";
         return false;
     }
     //栅格左下角是(-180,-90)到(179,89),左下角的经纬度是所在栅格所有经纬度的最小值。
     //在同一个栅格中向一个固定方向航行时，经度和纬度都应当单调变化
     //每个栅格的范围是[ )，即左闭右开。北极点(x,90)会遗漏
     lowerLeftLongitudeInDegree=colIndex*180.0/GRID_ARRAY_ROW_COUNT-180;
     lowerLeftLatidueInDegree=rowIndex*180.0/GRID_ARRAY_ROW_COUNT-90;

     if(lowerLeftLongitudeInDegree<-180||lowerLeftLongitudeInDegree>180)
         qDebug()<<"error in getlocation"<<lowerLeftLongitudeInDegree<<colIndex<<rowIndex;

     return true;
 }

 bool ParallelWorld::getGridIndex(const double &longitudeInDegree,const double &latitudeInDegree,
                                  qint32 &rowIndex, qint32 &colIndex) const
 {
     if(longitudeInDegree>180||longitudeInDegree<-180||
             latitudeInDegree>90||latitudeInDegree<-90)
     {
         /*
         emit sigShowInfo("Longitude and latitude is out of range. Lon:"+
                          QByteArray::number(longitudeInDegree)+" Lat:"
                 +QByteArray::number(latitudeInDegree));
                 */
         return false;
     }
     else
     {
         //每个栅格的范围是左闭右开
         rowIndex=qFloor( (latitudeInDegree+90)*GRID_ARRAY_ROW_COUNT/180.0);
         colIndex=qFloor( (longitudeInDegree+180)*GRID_ARRAY_ROW_COUNT/180.0);
         if(rowIndex>=(qint32)rowCount) //最大的一行和最大的一列特殊判断
             rowIndex=rowCount-1;
         if(colIndex>=(qint32)colCount)
             colIndex=colCount-1;
         return true;
     }
 }


 /****reply to PBMoniitor***/
 void ParallelWorld::slotPBMonitor(PBMonitor pbMonitor)
 {

     updateMonitorProbeAckWithOneMessageRcvd();

     //qDebug()<<QDateTime::currentDateTime()<< ":Monitor message recvd.\n"<<QString::fromStdString(pbMonitor.DebugString());
     QList<StructDataAndKey> listProtoData;
     if(pbMonitor.has_monitorprobe())
     {
         PBMonitor pbMonitorToSend;
         pbMonitorToSend.set_recordutctime(QDateTime::currentDateTime().toTime_t());
         pbMonitorToSend.set_sequencenum(pbCoderDecoder->getSerialNumAndIncrement());
         pbMonitorToSend.set_enum_sender_software(pbCoderDecoder->getPbEnumSenderSoftware());
         pbMonitorToSend.set_softwarestartedutctime(pbCoderDecoder->getStartedTimeUTC());

         PBMonitor_ProbeAck *probeAck=new PBMonitor_ProbeAck();
         probeAck->CopyFrom(monitor_ProbeAck);
         //probeAck->set_recordutctime(QDateTime::currentDateTime().toTime_t());
         probeAck->set_enum_probesender_software(pbMonitor.enum_sender_software());
         probeAck->set_monitorprobesequencenumacked(pbMonitor.monitorprobe().monitorprobesequencenum());
         pbMonitorToSend.set_allocated_monitorprobeack(probeAck);

         QByteArray baResult;
         baResult.resize(pbMonitorToSend.ByteSize());
         pbMonitorToSend.SerializeToArray(baResult.data(),pbMonitorToSend.ByteSize());
         StructDataAndKey dataAndKey;
         dataAndKey.data=baResult;
         dataAndKey.routingKey=ROUTING_KEY_MONITOR_PROBEACK;
         listProtoData.append(dataAndKey);
     }
     if(!listProtoData.isEmpty())
     {
         emit sigSend2MQ(listProtoData);
         updateMonitorProbeAckWithMessagesSent(listProtoData.size());
     }
 }

 void ParallelWorld::initWaterGrids()
 {
     for(qint32 rowIndex=0;rowIndex<(qint32)rowCount;rowIndex++)
     {
         for(qint32 colIndex=0;colIndex<(qint32)colCount;colIndex++)
         {




         }
     }
 }

bool ParallelWorld::isInWater(const double &longitudeInDegree,const double &latitudeInDegree)
{
    qint32 rowInd,colInd;
     if(getGridIndex(longitudeInDegree,latitudeInDegree,rowInd,colInd))
     {
         if(rowInd<(qint32)rowCount&&colInd<(qint32)colCount)
             return isWater[rowInd][colCount];
         else
             return false;
     }
     else
         return false;
}

 void ParallelWorld::updateMonitorProbeAckWithOneMessageRcvd()
 {
     monitor_ProbeAck.set_commandmessagesrcvd(monitor_ProbeAck.commandmessagesrcvd()+1);
     monitor_ProbeAck.set_recordutctime(QDateTime::currentDateTime().toTime_t());
 }

 void ParallelWorld::updateMonitorProbeAckWithOneMessageSent()
 {
     monitor_ProbeAck.set_commandmessagessent(monitor_ProbeAck.commandmessagessent()+1);
     monitor_ProbeAck.set_recordutctime(QDateTime::currentDateTime().toTime_t());
 }

 void ParallelWorld::updateMonitorProbeAckWithMessagesSent(qint32 messageCount)
 {
     monitor_ProbeAck.set_commandmessagessent(monitor_ProbeAck.commandmessagessent()+messageCount);
     monitor_ProbeAck.set_recordutctime(QDateTime::currentDateTime().toTime_t());
 }

 QMap<PB_Enum_TargetInfo_Type, Struct_PosDeviceInfo> ParallelWorld::getMapInfoTypePosDeviceInfo() const
 {
     return mapInfoTypePosDeviceInfo;
 }

 QHash<qint32, Target *> ParallelWorld::getHashIDTarget() const
 {
     return hashIDTarget;
 }

