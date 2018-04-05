#ifndef DATACHANNEL_H
#define DATACHANNEL_H

#include <QObject>
#include <QVector>
#include "Target.pb.h"
class ThreadedWorld;

class DataChannel : public QObject
{
    Q_OBJECT
public:
    explicit DataChannel(ThreadedWorld *world,  const PB_Enum_TargetInfo_Type   &targetInfoType,QObject *parent = 0);
    bool fetchDataFromPosDevicesIntoChannel(const qint64 &currentDateTimeMSecs);
    bool clearListPBTargetPosInChannel();

    QVector<PBTargetPosition> getVectPBTargetPosInChannel() const;

    PB_Enum_TargetInfo_Type getTargetInfoType() const;

signals:

public slots:

private:
    PB_Enum_TargetInfo_Type targetInfoType;
    ThreadedWorld *world;
    QVector <PBTargetPosition> vectPBTargetPosInChannel;
};

#endif // DATACHANNEL_H
