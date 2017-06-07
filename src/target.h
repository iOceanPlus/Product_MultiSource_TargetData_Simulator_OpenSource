#ifndef TARGET_H
#define TARGET_H

#include <QObject>
#include <QMultiMap>
#include "PBCoderDecoder.h"
#include "ContainerOfThreadMQTopicPublish.h"
#include "PosDevice.h"

const quint32 DegreesX10_ToTurn_WhenMeetLand=450;
class ParallelWorld;

class Target
{
public:
    explicit Target(const QHash <PB_Enum_TargetInfo_Type, PosDevice*> &hashTargetInfoTypePosDevice,
                             const PBTargetPosition &pbTargetPosOrig, ParallelWorld *parallelWorld, const QDateTime &posOrigDateTime);
    ~Target();

public:
    quint64 getTargetIDOrigAggregatedWithIDType(const quint8 &targetID_Type, const quint32 &targetIDOrig);
    PBTargetPosition getReckonedPbTargetPos(const QDateTime &dtToReckon, bool &isOnLand) const;

    PBTargetPosition updateAndGetPbTargetPosCurrent();

    QHash<PB_Enum_TargetInfo_Type, PosDevice *> getHashTargetInfoTypePosDevice() const;
    PosDevice* getDevice(const PB_Enum_TargetInfo_Type &infoType);
    const Struct_PosDeviceInfo getDeviceInfo(const PB_Enum_TargetInfo_Type &infoType) const;

    /************Update the pbTargetPosOrig when the target meet land **************/
    void  updateTargetPosCurrentAndOrigIfMeetLand();

    void setOriginalTargetIDsOfTargetPos(PBTargetPosition &pbTargetPosToSet);
    void  clearInvalidFields(PBTargetPosition &pbTargetPos);
    void set_enum_targetidorigAndIDType_AccordingToInfoType(PBTargetPosition &pbTargetPosToSet);


private:
    QHash <PB_Enum_TargetInfo_Type, PosDevice*> hashTargetInfoTypePosDevice;

    /*******
     * Periodically update pbTargetPosCurrent, assuming Great Circle journey from pbTargetPosOrig.
     * When the target meets land, change the pbTargetPosOrig,  pbTargetPosCurrent and posTime.
********/
    PBTargetPosition pbTargetPosOrig, pbTargetPosCurrent;
    QDateTime posOrigDateTime, posCurrentDateTime;

    //PBCoderDecoder *pbCoderDecoder;
    ParallelWorld *parallelWorld;
};

#endif // TARGET_H
