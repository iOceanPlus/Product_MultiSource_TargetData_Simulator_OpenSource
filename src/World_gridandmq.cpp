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
#include "macro.h"
#include "target.h"

bool ThreadedWorld::getLocation(quint32 rowIndex, quint32 colIndex,
                         double &lowerLeftLongitudeInDegree, double &lowerLeftLatidueInDegree)
{
    if(rowIndex>=GRID_ARRAY_ROW_COUNT||colIndex>=GRID_ARRAY_ROW_COUNT*2) //超出范围
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

bool ThreadedWorld::getGridIndex(const double &longitudeInDegree,const double &latitudeInDegree,
                                 qint32 &rowIndex, qint32 &colIndex)
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
        if(rowIndex>=(qint32)GRID_ARRAY_ROW_COUNT) //最大的一行和最大的一列特殊判断
            rowIndex=GRID_ARRAY_ROW_COUNT-1;
        if(colIndex>=(qint32)GRID_ARRAY_ROW_COUNT*2)
            colIndex=GRID_ARRAY_ROW_COUNT*2-1;
        return true;
    }
}


/****reply to PBMoniitor***/
void ThreadedWorld::slotPBMonitor(PBMonitor pbMonitor)
{
    updateTotalMsgOfProbeAckWithOneMessageRcvd();

    //qDebug()<<QDateTime::currentDateTime()<< ":Monitor message recvd.\n"<<QString::fromStdString(pbMonitor.DebugString());
    QList<StructDataAndKey> listProtoData;
    if(pbMonitor.has_monitorprobe())
    {
        PBMonitor pbMonitorToSend;
        pbMonitorToSend.set_recordutctime(QDateTime::currentDateTime().toTime_t());
        pbMonitorToSend.set_sequencenum(sharedPbCoderDecoder->getSerialNumAndIncrement());
        pbMonitorToSend.set_enum_sender_software(sharedPbCoderDecoder->getPbEnumSenderSoftware());
        pbMonitorToSend.set_softwarestartedutctime(sharedPbCoderDecoder->getStartedTimeUTC());

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
        updateTotalMsgOfMonitorProbeAckWithMessagesSent(listProtoData.size());
    }
}

void ThreadedWorld::addPreprocessedMsgsSendInMonitorProbeAck(const qint32 &preprocessedMsgsSent)
{
    monitor_ProbeAck.set_preprocessedtargetpositionssent(monitor_ProbeAck.preprocessedtargetpositionssent()+preprocessedMsgsSent);
    monitor_ProbeAck.set_totalmessagessent(monitor_ProbeAck.totalmessagessent()+preprocessedMsgsSent);
    monitor_ProbeAck.set_recordutctime(QDateTime::currentDateTime().toTime_t());
}


bool ThreadedWorld::isInWaterAndBoudingArea(const double &longitudeInDegree,const double &latitudeInDegree)
{
   qint32 rowInd,colInd;
    if(getGridIndex(longitudeInDegree,latitudeInDegree,rowInd,colInd))
    {
        if(rowInd<(qint32)GRID_ARRAY_ROW_COUNT&&colInd<(qint32)GRID_ARRAY_ROW_COUNT*2)
        {
            bool inWater= externVIsWater[rowInd][colInd];
            bool inBoundingArea=true;
            if(sharedGeoPolyGonBoundingRegion)
            {
                inBoundingArea=sharedGeoPolyGonBoundingRegion->containsPoint(QGeoCoordinate(latitudeInDegree,longitudeInDegree));
            }
            return inWater&&inBoundingArea;
        }
        else
            return false;
    }
    else
        return false;
}

void ThreadedWorld::updateTotalMsgOfProbeAckWithOneMessageRcvd()
{
    monitor_ProbeAck.set_totalmessagesrcvd(monitor_ProbeAck.totalmessagesrcvd()+1);
    monitor_ProbeAck.set_recordutctime(QDateTime::currentDateTime().toTime_t());
}

void ThreadedWorld::updateTotalMsgOfMonitorProbeAckWithMessagesSent(qint32 messageCount)
{
    monitor_ProbeAck.set_totalmessagessent(monitor_ProbeAck.totalmessagessent()+messageCount);
    monitor_ProbeAck.set_recordutctime(QDateTime::currentDateTime().toTime_t());
}
