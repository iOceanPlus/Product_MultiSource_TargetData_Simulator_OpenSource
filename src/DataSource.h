#ifndef DATASOURCE_H
#define DATASOURCE_H

#include <QObject>
#include <QMap>
#include "CommonEnum.pb.h"
#include "IOMessages.h"

class ParallelWorld;
struct Struct_TransmitQuality
{
    PB_Enum_TargetInfo_Type infoType;
    double meanLatencyInSeconds;
    double stdDevLatencyInSeconds;
    quint8 packetLossPercentage;
};

class DataSource : public QObject
{
    Q_OBJECT
public:
    explicit DataSource(ParallelWorld *world,const QMap <PB_Enum_TargetInfo_Type,Struct_TransmitQuality>  &mapInfoTypeTransmitQuality,
                        QObject *parent = 0);
    QList <StructDataAndKey> fetchDataFromChannels();

signals:

public slots:

private:
    QMap <PB_Enum_TargetInfo_Type,Struct_TransmitQuality> mapInfoTypeTransmitQuality;
    ParallelWorld *world;
};

#endif // DATASOURCE_H
