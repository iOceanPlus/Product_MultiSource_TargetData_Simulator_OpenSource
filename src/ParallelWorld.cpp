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

ParallelWorld::ParallelWorld(QMap<quint64, qint32> mapClockIDSecondsAhead, QMap<quint64, qint32> mapClockIDSecondsAheadStddev,
                             qint32 secondsWindowSizeOfTargetPos,  bool paramIsChosenWorld, bool paramFiltOutlierTimeRecord, QMutex *mutex,  QObject *parent) :
                            QObject(parent)
{
    colCount=GRID_ARRAY_ROW_COUNT*2;
    rowCount=GRID_ARRAY_ROW_COUNT;


    this->mutex=mutex;

    //pbCoderDecoderForAggregatedPBToSend=new PBCoderDecoder(SOFTWARE_NAME,this);
    pbCoderDecoder=new PBCoderDecoder(SOFTWARE_NAME,this);

    initiateWorldGrids();
}

ParallelWorld::~ParallelWorld()
{

}

 void  ParallelWorld::slotPutInitialTargetsToGrid(QList <PBTargetPosition> listPbTargetPos)
{
    QListIterator <PBTargetPosition> iListPos(listPbTargetPos);
     while(iListPos.hasNext())
     {
         PBTargetPosition targetPosInList=iListPos.next();

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

 void ParallelWorld::initiateWorldGrids()
 {
     for(qint32 rowIndex=0;rowIndex<(qint32)rowCount;rowIndex++)
     {
         for(qint32 colIndex=0;colIndex<(qint32)colCount;colIndex++)
         {




         }
     }
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


