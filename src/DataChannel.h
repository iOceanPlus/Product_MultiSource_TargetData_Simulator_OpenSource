#ifndef DATACHANNEL_H
#define DATACHANNEL_H

#include <QObject>
#include "Target.pb.h"
class ParallelWorld;

class DataChannel : public QObject
{
    Q_OBJECT
public:
    explicit DataChannel(ParallelWorld *world,  const PB_Enum_TargetInfo_Type   &targetInfoType,QObject *parent = 0);
    QList <PBTargetPosition> fetchDataFromPosDevices();

signals:

public slots:

private:
    PB_Enum_TargetInfo_Type targetInfoType;
    ParallelWorld *world;
};

#endif // DATACHANNEL_H
