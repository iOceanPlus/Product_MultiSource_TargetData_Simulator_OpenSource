#ifndef POSDEVICE_H
#define POSDEVICE_H

#include "CommonEnum.pb.h"
#include "Target.pb.h"
#include <QObject>
#include <QDateTime>
class Target;
class ThreadedWorld;

struct Struct_PosDeviceInfo
{
    PB_Enum_TargetInfo_Type infoType;
    qint64 sampleMilliSeconds;
    double positioningDevInMeters;
};

/****** Positioning devices like AIS, radar, beidou, etc. *******/
class PosDevice
{
public:
    PosDevice(const QDateTime &lastDeviceSampleTime, Target * const target, const PB_Enum_TargetInfo_Type &infoType);
    /*** If the time since last measurement is no less than sampling interval, the measure will be successful*****/
    PBTargetPosition measurePBTargetPosAndUpdateTarget(bool &isMeasureSuccessful);
    bool addDevToPos(PBTargetPosition &pbTargetPos);

private:    
    PB_Enum_TargetInfo_Type infoType;
    Target *targetInstalled;
    QDateTime lastSampleTime;
};

#endif // POSDEVICE_H
