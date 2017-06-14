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
#include "PosDevice.h"

using namespace std;

class PBCoderDecoder;
class Target;
class QTimer;

struct GridIndex
{
    qint32 rowIndex;
    qint32 colIndex;
};

class World : public QObject
{
    Q_OBJECT
public:
    explicit World( QMutex *mutex, QObject *parent = 0);
    ~World();
    bool isInWater(const double &longitudeInDegree,const double &latitudeInDegree);

    QHash<qint32, Target *> getHashIDTarget() const;

    QMap<PB_Enum_TargetInfo_Type, Struct_PosDeviceInfo> getMapInfoTypePosDeviceInfo() const;

    QMap<PB_Enum_TargetInfo_Type, DataChannel *> getMapInfoTypeDataChannels() const;

    PBCoderDecoder *getPbCoderDecoder() const;
    void addPreprocessedMsgsSendInMonitorProbeAck(const qint32 &preprocessedMsgsSent);

signals:
    void sigSend2MQ(QList <StructDataAndKey> listProtoData);
    void sigPBMonitor(PBMonitor pbMonitor); //

private slots:
    void slotPBMonitor(PBMonitor pbMonitor); //
    void slotTimerEventMeasureAndUpdateTargetsPos();
    void slotTimerEventOutPutTargetCountAndMsgRate();
private:
    void parseParamFileAndInitMembers();

    void initTargetsAndAddToDataSources();
    void initDataChannels();
    void initWaterGrids();

    bool checkJsonObjectAndOutPutValue(const QJsonObject &jsonObject,  const QString &key, const bool &outPutValue);

    /****************************************Grids related methods***********************************/
    bool getLocation(quint32 rowIndex, quint32 colIndex,double &lowerLeftLongitudeInDegree, double &lowerLeftLatidueInDegree);
    bool getGridIndex(const double &longitudeInDegree,const double &latitudeInDegree,
                      qint32 &rowIndex, qint32 &colIndex) const;

    void updateTotalMsgOfProbeAckWithOneMessageRcvd();
    void updateTotalMsgOfMonitorProbeAckWithMessagesSent(qint32 messageCount);

    void sendPBTargetToMQ(const PBTargetPosition &pbTargetPosToSend);

    QHash <qint32, Target*> hashIDTarget;
    QMap <PB_Enum_TargetInfo_Source,DataSource*> mapInfoSourceDataSources;

    QMap <PB_Enum_TargetInfo_Type,DataChannel*> mapInfoTypeDataChannels;

    QMap <PB_Enum_TargetInfo_Type, Struct_PosDeviceInfo> mapInfoTypePosDeviceInfo;

    bool isWater[GRID_ARRAY_ROW_COUNT][2*GRID_ARRAY_ROW_COUNT]; //栅格

    quint32 colCount,rowCount;
    PBCoderDecoder *pbCoderDecoder;  //*pbCoderDecoderForAggregatedPBToSend;
    QString waterGridsFileName,ship_FileName, mc2FileName, language;

    QTimer *timerMeasureAndUpdateTargetPos;
    QTimer *timerOutputTargetCountAndMsgRate;

    PBMonitor_ProbeAck monitor_ProbeAck;
    QMutex *mutex;
};

#endif // PARALLELWORLD_H
