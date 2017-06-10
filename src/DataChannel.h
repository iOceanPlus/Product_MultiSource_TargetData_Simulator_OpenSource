#ifndef DATACHANNEL_H
#define DATACHANNEL_H

#include <QObject>
#include "Target.pb.h"
class World;

class DataChannel : public QObject
{
    Q_OBJECT
public:
    explicit DataChannel(World *world,  const PB_Enum_TargetInfo_Type   &targetInfoType,QObject *parent = 0);
    bool fetchDataFromPosDevicesIntoChannel();
    bool clearListPBTargetPosInChannel();

    QList<PBTargetPosition> getListPBTargetPosInChannel() const;

    PB_Enum_TargetInfo_Type getTargetInfoType() const;

signals:

public slots:

private:
    PB_Enum_TargetInfo_Type targetInfoType;
    World *world;
    QList <PBTargetPosition> listPBTargetPosInChannel;
};

#endif // DATACHANNEL_H
