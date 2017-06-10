#ifndef DATASOURCE_H
#define DATASOURCE_H

#include <QObject>
#include <QMap>
#include <QHash>
#include <QSet>
#include <QDateTime>
#include "CommonEnum.pb.h"
#include "IOMessages.h"

class World;
class Target;
class QTimer;

struct Struct_TransmissionQuality
{
    PB_Enum_TargetInfo_Type infoType;
    qint32 meanTransmissionLatencyInMiliSeconds; //The latency of transmission
    qint32 stdDevTransmissionLatencyInMiliSeconds;
    qint32 meanTimestampErrorInMiliSeconds; //Error in the timestamp of PBTargetPosition
    qint32 stdDevTimestampErrorInMiliSeconds; //
    quint8 packetLossPercentage; //When data souce fetch data from Data Channel, some packets are lost
    quint8 percentageTargetsObserved; //After initialization, which targets this source can observe is fixed.
};


class DataSource : public QObject
{
    Q_OBJECT
public:
    explicit DataSource(World *world, const PB_Enum_TargetInfo_Source &pbTargetInfoSource,
                        const QMap <PB_Enum_TargetInfo_Type,Struct_TransmissionQuality>  &mapInfoTypeTransmitQuality, QObject *parent = 0);

    bool addTargetIDObservedWithAIS(qint32 targetID);
    bool addTargetIDObservedWithBeidou(qint32 targetID);
    bool addTargetIDObservedWithArgosAndMarineSat(qint32 targetID);
    bool addTargetIDObservedWithHaijian(qint32 targetID);

    bool fetchDataFromChannelsAndSendToMQ();

    QMap<PB_Enum_TargetInfo_Type, Struct_TransmissionQuality> getMapInfoTypeTransmitQuality() const;
    PB_Enum_TargetInfo_Source getPbTargetInfoSource() const;

signals:
    void sigSend2MQ(QList <StructDataAndKey> listProtoData);

public slots:
    void slotOutPutTargetsCountPerType();
    void slotOutPutPosCountAndRate();

private:
    bool fetchDataFromAChannel(const PB_Enum_TargetInfo_Type &targetInfoType, QList <StructDataAndKey> &listDataAndKey,
                               QSet <qint32> &setTargetIDObservedOfThisInfoType,QSet <qint32> &setTargetIDSentOfThisInfoType, const QString &routingKey);

    void addTimeStampErrorInDynamicOfTargetPos(PBTargetPosition &pbTargetPos, const Struct_TransmissionQuality &transQ) const;

    QMap <PB_Enum_TargetInfo_Type,Struct_TransmissionQuality> mapInfoTypeTransmitQuality;
    World *world;

    QSet <qint32> setTargetIDsObservedWithAIS, setTargetIDsObservedWithBeidou,setTargetIDsObservedWithArgosAndMarineSat,
                setTargetIDsObservedWithHaijian; //Which targets this data source is able to observe.
    QSet <qint32> setTargetIDsSentWithAIS, setTargetIDsSentWithBeidou,setTargetIDsSentWithArgosAndMarineSat,
                setTargetIDsSentWithHaijian;     //Which targets have been sent by this data source.
    PB_Enum_TargetInfo_Source pbTargetInfoSource;

    quint64 totalPosCountFetched, posCountOutputToLog;
    QDateTime dtPosCountFetched, dtPosCountOutputToLog;

    QTimer *timerOutPutInfo;
};

#endif // DATASOURCE_H
