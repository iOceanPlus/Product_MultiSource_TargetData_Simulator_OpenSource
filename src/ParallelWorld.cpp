#include "ParallelWorld.h"
#include <QMutex>
#include <QDebug>
#include <QTimer>
#include <QtMath>
#include <exception>
#include <QDateTime>
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


    initWaterGrids();
    initTargets();
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

 QHash<qint32, Target *> ParallelWorld::getHashIDTarget() const
 {
     return hashIDTarget;
 }

