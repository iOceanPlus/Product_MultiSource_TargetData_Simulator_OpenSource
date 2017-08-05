#ifndef DATACHANNEL_H
#define DATACHANNEL_H

#include <QObject>
#include "Target.pb.h"
class ThreadedWorld;

class DataChannel : public QObject
{
    Q_OBJECT
public:
    explicit DataChannel(ThreadedWorld *world,  const PB_Enum_TargetInfo_Type   &targetInfoType,QObject *parent = 0);
    bool fetchDataFromPosDevicesIntoChannel();
    bool clearListPBTargetPosInChannel();

    QList<PBTargetPosition> getListPBTargetPosInChannel() const;

    PB_Enum_TargetInfo_Type getTargetInfoType() const;

signals:

public slots:

private:
    PB_Enum_TargetInfo_Type targetInfoType;
    ThreadedWorld *world;
    QList <PBTargetPosition> listPBTargetPosInChannel;
};

#endif // DATACHANNEL_H
