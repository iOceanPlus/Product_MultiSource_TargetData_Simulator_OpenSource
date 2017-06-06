#include "DataChannel.h"
#include "ParallelWorld.h"
#include "target.h"

DataChannel::DataChannel(ParallelWorld *world, const PB_Enum_TargetInfo_Type &targetInfoType, QObject *parent) : QObject(parent)
{
    this->targetInfoType=targetInfoType;
    this->world=world;
}

QList <PBTargetPosition> DataChannel::fetchDataFromPosDevices()
{
    QList <PBTargetPosition> listPBTargetPosGot;
    QListIterator <Target*> iListTarget(world->getListTargets());
    while(iListTarget.hasNext())
    {
        Target *target=iListTarget.next();

        bool isMeasured;
        PosDevice *posDevice= target->getDevice(targetInfoType);
        if(posDevice)
        {
            PBTargetPosition pbTargetPosMeasured= target->getDevice(targetInfoType)->measurePBTargetPosAndUpdateTarget(isMeasured);
            if(isMeasured)
                listPBTargetPosGot.append(pbTargetPosMeasured);
        }
    }
    return listPBTargetPosGot;
}
