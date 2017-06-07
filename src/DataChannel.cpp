#include "DataChannel.h"
#include "ParallelWorld.h"
#include "target.h"

DataChannel::DataChannel(ParallelWorld *world, const PB_Enum_TargetInfo_Type &targetInfoType, QObject *parent) : QObject(parent)
{
    this->targetInfoType=targetInfoType;
    this->world=world;
}

bool DataChannel::fetchDataFromPosDevicesIntoChannel()
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
            PBTargetPosition pbTargetPosMeasured= target->getDevice(targetInfoType)->measurePBTargetPosAndUpdateTarget(isMeasured);
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
