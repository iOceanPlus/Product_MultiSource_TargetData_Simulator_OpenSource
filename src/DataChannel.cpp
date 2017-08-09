#include "DataChannel.h"
#include "ThreadedWorld.h"
#include "target.h"
#include <QDebug>

DataChannel::DataChannel(ThreadedWorld *world, const PB_Enum_TargetInfo_Type &targetInfoType, QObject *parent) : QObject(parent)
{
    this->targetInfoType=targetInfoType;
    this->world=world;
}

bool DataChannel::fetchDataFromPosDevicesIntoChannel(const qint64 &currentDateTimeMSecs)
{
    QHashIterator <qint32, Target*> iHashTarget(world->getHashIDTarget());
    while(iHashTarget.hasNext())
    {
        iHashTarget.next();
        Target *target=iHashTarget.value();
        bool isMeasured;
        PosDevice *posDevice= target->getDevice(targetInfoType);
        if(posDevice)
        {
            PBTargetPosition pbTargetPosMeasured= posDevice->measurePBTargetPosAndUpdateTarget(isMeasured,currentDateTimeMSecs);
            if(isMeasured)
                listPBTargetPosInChannel.append(pbTargetPosMeasured);
        }
    }
    return true;
}

bool DataChannel::clearListPBTargetPosInChannel()
{
    listPBTargetPosInChannel.clear();
    return true;
}

QList<PBTargetPosition> DataChannel::getListPBTargetPosInChannel() const
{
    return listPBTargetPosInChannel;
}

PB_Enum_TargetInfo_Type DataChannel::getTargetInfoType() const
{
    return targetInfoType;
}
