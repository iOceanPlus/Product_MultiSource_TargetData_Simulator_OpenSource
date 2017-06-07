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
#include "DataChannel.h"
#include "DataSource.h"

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
    explicit ParallelWorld( QMutex *mutex, QObject *parent = 0);
    ~ParallelWorld();
    bool isInWater(const double &longitudeInDegree,const double &latitudeInDegree);

    QHash<qint32, Target *> getHashIDTarget() const;

signals:
    void sigSend2MQ(QList <StructDataAndKey> listProtoData);

private slots:
    void slotPBMonitor(PBMonitor pbMonitor); //
    void slotTimerEventMeasureAndUpdateTargetsPos();
private:
    void parseParamFileAndInitMembers();

    void initTargets();
    void initDataSources();
    void initDataChannels();
    void initWaterGrids();

    bool checkJsonObjectAndOutPutValue(const QJsonObject &jsonObject,  const QString &key);

    /****************************************Grids related methods***********************************/
    bool getLocation(quint32 rowIndex, quint32 colIndex,double &lowerLeftLongitudeInDegree, double &lowerLeftLatidueInDegree);
    bool getGridIndex(const double &longitudeInDegree,const double &latitudeInDegree,
                      qint32 &rowIndex, qint32 &colIndex) const;

    void updateMonitorProbeAckWithOneMessageRcvd();
    void updateMonitorProbeAckWithOneMessageSent();
    void updateMonitorProbeAckWithMessagesSent(qint32 messageCount);

    void sendPBTargetToMQ(const PBTargetPosition &pbTargetPosToSend);

    QHash <qint32, Target*> hashIDTarget;
    QMap <PB_Enum_TargetInfo_Type,DataChannel*> mapInfoTypeDataChannels;
    QMap <PB_Enum_TargetInfo_Type,DataSource*> mapInfoTypeDataSources;

    bool isWater[GRID_ARRAY_ROW_COUNT][2*GRID_ARRAY_ROW_COUNT]; //栅格

    quint32 colCount,rowCount;
    PBCoderDecoder *pbCoderDecoder;  //*pbCoderDecoderForAggregatedPBToSend;

    PBMonitor_ProbeAck monitor_ProbeAck;
    QMutex *mutex;
};

#endif // PARALLELWORLD_H
