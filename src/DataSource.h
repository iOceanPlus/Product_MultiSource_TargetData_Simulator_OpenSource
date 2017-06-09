#ifndef DATASOURCE_H
#define DATASOURCE_H

#include <QObject>
#include <QMap>
#include <QHash>
#include <QSet>
#include "CommonEnum.pb.h"
#include "IOMessages.h"

class ParallelWorld;
class Target;

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
    explicit DataSource(ParallelWorld *world, const PB_Enum_TargetInfo_Source &pbTargetInfoSource,
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

private:
    bool fetchDataFromAChannel(const PB_Enum_TargetInfo_Type &targetInfoType, QList <StructDataAndKey> &listDataAndKey,
                               QSet <qint32> &setTargetIDObservedOfThisInfoType);

    void addTimeStampErrorInDynamicOfTargetPos(PBTargetPosition &pbTargetPos, const Struct_TransmissionQuality &transQ) const;

    QMap <PB_Enum_TargetInfo_Type,Struct_TransmissionQuality> mapInfoTypeTransmitQuality;
    ParallelWorld *world;
    QSet <qint32> setTargetIDsObservedWithAIS, setTargetIDsObservedWithBeidou,setTargetIDsObservedWithArgosAndMarineSat,
                setTargetIDsObservedWithHaijian;
    PB_Enum_TargetInfo_Source pbTargetInfoSource;
};

#endif // DATASOURCE_H
