﻿#ifndef THREADEDWORLD_H
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
    explicit ThreadedWorld( QMutex *sharedMutex, MyQtGeoPolygon *sharedGeoPolyGonBoundingRegion,PBCoderDecoder *pbCodDecoder,
                            quint16 worldIndex,const QString& language,  QObject *parent = 0);
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

    /****************************************Grids related methods***********************************/
    static bool getLocation(quint32 rowIndex, quint32 colIndex,double &lowerLeftLongitudeInDegree, double &lowerLeftLatidueInDegree);
    static bool getGridIndex(const double &longitudeInDegree,const double &latitudeInDegree,
                      qint32 &rowIndex, qint32 &colIndex);
    QMap<PB_Enum_TargetInfo_Source, DataSource *> getMapInfoSourceDataSources() const;

    void updateTargetCountAndMsgRate(QSet<qint32> &setDistinctTargetIDOrig, QMap<PB_Enum_TargetInfo_Type,
                           QSet<qint32> > &mapInfoTypeSetTargetID, qint32 &targetCountAll,
                                                   float &msgCountPerMinuteCount, quint64 &messageCountSum);
    std::default_random_engine *getRandomEngine() const;

    QString getLanguage() const;

signals:
    void sigSend2MQ(QList <StructDataAndKey> listProtoData);
    void sigPBMonitor(PBMonitor pbMonitor); //
public slots:
    bool slotAddDataSourceIfNotExist(const PB_Enum_TargetInfo_Source &pbTargetInfoSource,
                                 const QMap <PB_Enum_TargetInfo_Type,Struct_TransmissionQuality>  &mapInfoTypeTransmitQuality);
    bool slotCreateTargets(const QList<PBTargetPosition> &listPbTargetPos, const quint16 &worldCount);
    void slotInitDataChannels(const QMap<PB_Enum_TargetInfo_Type, Struct_PosDeviceInfo> &mapInfoTypePosDeviceInfo);

private slots:
    void slotPBMonitor(PBMonitor pbMonitor); //
    void slotTimerEventMeasureAndUpdateTargetsPos();

private:
    void updateTotalMsgOfProbeAckWithOneMessageRcvd();
    void updateTotalMsgOfMonitorProbeAckWithMessagesSent(qint32 messageCount);

    void sendPBTargetToMQ(const PBTargetPosition &pbTargetPosToSend);

    QHash <qint32, Target*> hashIDTarget;
    QMap <PB_Enum_TargetInfo_Source,DataSource*> mapInfoSourceDataSources;
    QMap <PB_Enum_TargetInfo_Type,DataChannel*> mapInfoTypeDataChannels;

    PBCoderDecoder *sharedPbCoderDecoder;  //*pbCoderDecoder
    MyQtGeoPolygon *sharedGeoPolyGonBoundingRegion; //

    QTimer *timerMeasureAndUpdateTargetPos;

    QMutex *sharedMutex;
    quint16 worldIndex;

    QString language;
    std::default_random_engine *randomEngine;
};

#endif // THREADEDWORLD_H
