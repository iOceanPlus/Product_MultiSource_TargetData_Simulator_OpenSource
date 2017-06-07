#ifndef DATASOURCE_H
#define DATASOURCE_H

#include <QObject>
#include <QMap>
#include "CommonEnum.pb.h"
#include "IOMessages.h"

class ParallelWorld;
struct Struct_TransmissionQuality
{
    PB_Enum_TargetInfo_Type infoType;
    qint32 meanTransmissionLatencyInMiliSeconds; //The latency of transmission
    qint32 stdDevTransmissionLatencyInMiliSeconds;
    qint32 meanTimestampErrorInMiliSeconds; //Error in the timestamp of PBTargetPosition
    qint32 stdDevTimestampErrorInMiliSeconds; //
    quint8 packetLossPercentage; //When data souce fetch data from Data Channel, some packets are lost
};


class DataSource : public QObject
{
    Q_OBJECT
public:
    explicit DataSource(ParallelWorld *world,const QMap <PB_Enum_TargetInfo_Type,Struct_TransmissionQuality>  &mapInfoTypeTransmitQuality,
                        QObject *parent = 0);
    bool fetchDataFromChannelsAndSendToMQ();

signals:

public slots:

private:
    QMap <PB_Enum_TargetInfo_Type,Struct_TransmissionQuality> mapInfoTypeTransmitQuality;
    ParallelWorld *world;
};

#endif // DATASOURCE_H
