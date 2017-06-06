#include <QDateTime>
#include "posdevice.h"
#include "target.h"
PosDevice::PosDevice(const PB_Enum_TargetInfo_Type   &targetInfoType, Target * const targetInstalled,
                     qint64 sampleMilliSeconds, QDateTime lastDeviceSampleTime, const double &positioningDevInMeters)
{
    this->targetInfoType=targetInfoType;
    this->targetInstalled=targetInstalled;
    this->sampleMilliSeconds=sampleMilliSeconds;
    this->positioningDevInMeters=positioningDevInMeters;
    this->lastSampleTime= lastDeviceSampleTime;
}

PBTargetPosition PosDevice::measurePBTargetPosAndUpdateTarget(bool &isMeasureSuccessful)
{
    QDateTime currentTime=QDateTime::currentDateTime();
    qint64 msecondsElapsed=lastSampleTime.msecsTo(currentTime);
    if(msecondsElapsed<sampleMilliSeconds)
    {
        isMeasureSuccessful=false;
        return PBTargetPosition.default_instance();
    }
    else
    {
        isMeasureSuccessful=true;
        targetInstalled->updateTargetPosCurrentAndOrigIfMeetLand();
        lastSampleTime=currentTime;
        return targetInstalled->get


    }

}
