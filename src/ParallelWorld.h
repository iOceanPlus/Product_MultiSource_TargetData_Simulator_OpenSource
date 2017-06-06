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

class ParallelWorld : public QObject
{
    Q_OBJECT
public:
    explicit ParallelWorld(QMap <quint64,qint32> mapClockIDSecondsAhead, QMap <quint64,qint32> mapClockIDSecondsAheadStddev,
                            qint32 secondsWindowSizeOfTargetPos, bool paramIsChosenWorld, bool paramFiltOutlierTimeRecord,QMutex *mutex, QObject *parent = 0);
    ~ParallelWorld();
    bool isInWater(const double &longitudeInDegree,const double &latitudeInDegree);

signals:
    void sigSend2MQ(QList <StructDataAndKey> listProtoData);

private slots:
    void slotPBMonitor(PBMonitor pbMonitor); //
private:
    void initTargets(QList <PBTargetPosition> listPbTargetPos);

    void updateMonitorProbeAckWithOneMessageRcvd();
    void updateMonitorProbeAckWithOneMessageSent();
    void updateMonitorProbeAckWithMessagesSent(qint32 messageCount);

    void sendPBTargetToMQ(const PBTarget &pbTargetToSend);
    void sendPBTargetToMQ(const PBTargetPosition &pbTargetPosToSend);
    void sendPBTargetToMQ(qint32 targetID);

    /****************************************Grids related methods***********************************/
    void initiateWaterGrids();
    bool getLocation(quint32 rowIndex, quint32 colIndex,double &lowerLeftLongitudeInDegree, double &lowerLeftLatidueInDegree);
    bool getGridIndex(const double &longitudeInDegree,const double &latitudeInDegree,
                      qint32 &rowIndex, qint32 &colIndex) const;

    bool isWater[GRID_ARRAY_ROW_COUNT][2*GRID_ARRAY_ROW_COUNT]; //栅格

    quint32 colCount,rowCount;
    PBCoderDecoder *pbCoderDecoder;  //*pbCoderDecoderForAggregatedPBToSend;

    PBMonitor_ProbeAck monitor_ProbeAck;

    QMutex *mutex;
};

#endif // PARALLELWORLD_H
