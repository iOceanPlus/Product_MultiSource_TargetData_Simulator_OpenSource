#ifndef PARALLELWORLD_H
#define PARALLELWORLD_H

#include <QObject>
#include <string>
#include <QMap>
#include <QHash>
#include <QTimer>
#include <QMutex>
#include "macro.h"
#include "Target.pb.h"
#include "Monitor.pb.h"
#include "IOMessages.h"

using namespace std;

class PBCoderDecoder;
class Target;

struct GridIndex
{
    qint32 rowIndex;
    qint32 colIndex;
};

/*********************
 * 说明：类中存储了所有栅格的指针，以及每个target位于哪个栅格中。
 * 在栅格类SpatialGrid中，记录了位于该栅格中的每个目标的指针。
 * ParallelWorld类可以根据MMSI查出哪个SpatialGrid中有这艘船，然后再去这个SpatialGrid中获取这艘船的指针 *
 *
 * ********************/
class ParallelWorld : public QObject
{
    Q_OBJECT
public:
    explicit ParallelWorld(QMap <quint64,qint32> mapClockIDSecondsAhead, QMap <quint64,qint32> mapClockIDSecondsAheadStddev,
                            qint32 secondsWindowSizeOfTargetPos, bool paramIsChosenWorld, bool paramFiltOutlierTimeRecord,QMutex *mutex, QObject *parent = 0);
    ~ParallelWorld();

signals:
     //Connect this signal with that of Association only if this is the chosen world
    void sigSend2MQ(QList <StructDataAndKey> listProtoData);

private slots:
    void slotPutInitialTargetsToGrid(QList <PBTargetPosition> listPbTargetPos);

    void slotPBMonitor(PBMonitor pbMonitor); //
private:

    void updateMonitorProbeAckWithOneMessageRcvd();
    void updateMonitorProbeAckWithOneMessageSent();
    void updateMonitorProbeAckWithMessagesSent(qint32 messageCount);

    void sendPBTargetToMQ(const PBTarget &pbTargetToSend);
    void sendPBTargetToMQ(const PBTargetPosition &pbTargetPosToSend);
    void sendPBTargetToMQ(qint32 targetID);

    /****************************************Grids related methods***********************************/
    void initiateWorldGrids();

    bool isWater[GRID_ARRAY_ROW_COUNT][2*GRID_ARRAY_ROW_COUNT]; //栅格

    quint32 colCount,rowCount;
    PBCoderDecoder *pbCoderDecoder;  //*pbCoderDecoderForAggregatedPBToSend;

    PBMonitor_ProbeAck monitor_ProbeAck;

    QMutex *mutex;
};

#endif // PARALLELWORLD_H
