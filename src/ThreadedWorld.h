#ifndef THREADEDWORLD_H
#define THREADEDWORLD_H

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
#include "MyQtGeoPolygon.h"

using namespace std;

class PBCoderDecoder;
class Target;
class QTimer;

struct GridIndex
{
    qint32 rowIndex;
    qint32 colIndex;
};

class ThreadedWorld : public QObject
{
    Q_OBJECT
public:
    explicit ThreadedWorld( QMutex *sharedMutex, MyQtGeoPolygon *sharedGeoPolyGonBoundingRegion,PBCoderDecoder *pbCodDecoder,  QObject *parent = 0);
    ~ThreadedWorld();
    bool isInWaterAndBoudingArea(const double &longitudeInDegree,const double &latitudeInDegree);

    QHash<qint32, Target *> getHashIDTarget() const;

    QMap<PB_Enum_TargetInfo_Type, Struct_PosDeviceInfo> getMapInfoTypePosDeviceInfo() const;

    QMap<PB_Enum_TargetInfo_Type, DataChannel *> getMapInfoTypeDataChannels() const;

    PBCoderDecoder *getPbCoderDecoder() const;
    void addPreprocessedMsgsSendInMonitorProbeAck(const qint32 &preprocessedMsgsSent);
#ifdef DEBUG_TargetCount
    QMultiMap <PB_Enum_TargetInfo_Type, qint32> multiMapInfoTypeOrigTargetIDForDebug;
#endif
    bool createOneTarget(qint32 &targetID, const PBTargetPosition &pbTargetPos,  const QDateTime &posOrigDateTime);

    QMap<PB_Enum_TargetInfo_Type, QSet<qint32> > & getRefMapInfoTypeSetTargetID() ;

signals:
    void sigSend2MQ(QList <StructDataAndKey> listProtoData);
    void sigPBMonitor(PBMonitor pbMonitor); //

private slots:
    void slotPBMonitor(PBMonitor pbMonitor); //
    void slotTimerEventMeasureAndUpdateTargetsPos();
    void slotTimerEventOutPutTargetCountAndMsgRate(QSet<qint32> &setDistinctTargetIDOrig);
private:

    void initTargetsAndAddToDataSources();
    void initDataChannels();
    void initWaterGrids();

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

    quint32 colCount,rowCount;
    PBCoderDecoder *sharedPbCoderDecoder;  //*pbCoderDecoder
    MyQtGeoPolygon *sharedGeoPolyGonBoundingRegion; //

    QTimer *timerMeasureAndUpdateTargetPos;
    QTimer *timerOutputTargetCountAndMsgRate;

    PBMonitor_ProbeAck monitor_ProbeAck;
    QMutex *sharedMutex;
};

#endif // THREADEDWORLD_H
