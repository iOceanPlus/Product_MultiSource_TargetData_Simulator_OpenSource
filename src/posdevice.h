#ifndef POSDEVICE_H
#define POSDEVICE_H

#include "CommonEnum.pb.h"
#include "Target.pb.h"
#include <QObject>
#include <QDateTime>
class Target;

/****** Positioning devices like AIS, radar, beidou, etc. *******/
class PosDevice
{
public:
    PosDevice(const PB_Enum_TargetInfo_Type   &targetInfoType, Target * const targetInstalled, qint64 sampleMilliSeconds,
                               QDateTime lastDeviceSampleTime, const double &positioningDevInMeters=10);
    /*** If the time since last measurement is no less than sampling interval, the measure will be successful*****/
    PBTargetPosition measurePBTargetPosAndUpdateTarget(bool &isMeasureSuccessful);

private:
     PB_Enum_TargetInfo_Type   targetInfoType;
     Target *targetInstalled;
     double positioningDevInMeters;
     qint64 sampleMilliSeconds;
     QDateTime lastSampleTime;
};

#endif // POSDEVICE_H
