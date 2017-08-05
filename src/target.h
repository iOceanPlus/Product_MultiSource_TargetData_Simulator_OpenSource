#ifndef TARGET_H
#define TARGET_H

#include <QObject>
#include <QMultiMap>
#include "PBCoderDecoder.h"
#include "ContainerOfThreadMQTopicPublish.h"
#include "PosDevice.h"
#include <QGeoCoordinate>

class ThreadedWorld;

class Target
{
public:
    explicit Target(const PBTargetPosition &pbTargetPosOrig, ThreadedWorld *world, const QDateTime &posOrigDateTime);
    bool addPosDevice(PB_Enum_TargetInfo_Type infoType, PosDevice* posDev);
    bool installPosDevices();
    ~Target();

    quint64 getTargetIDOrigAggregatedWithIDType(const quint8 &targetID_Type, const quint32 &targetIDOrig);

    /************Update the pbTargetPosOrig when the target meet land **************/
    PBTargetPosition updateAndGetPbTargetPosCurrent();

    QHash<PB_Enum_TargetInfo_Type, PosDevice *> getHashTargetInfoTypePosDevice() const;
    PosDevice* getDevice(const PB_Enum_TargetInfo_Type &infoType);
    const Struct_PosDeviceInfo getDeviceInfo(const PB_Enum_TargetInfo_Type &infoType) const;

    void setOriginalTargetIDsOfTargetPos(PBTargetPosition &pbTargetPosToSet);
    void  clearInvalidFieldsInAnOriginalTargetPos(PBTargetPosition &pbTargetPos);
    void set_enum_targetidorigAndIDType_AccordingToInfoType(PBTargetPosition &pbTargetPosToSet);

private:
    void updatePosAndCOG(const QDateTime &dtReckoned, const QGeoCoordinate &geoReckoned) ;
    const QGeoCoordinate getConstCurrentGeoPosHighPreciReckoned(const QGeoCoordinate &geoOrig, const  QDateTime &dtOrig, const double &sogKnotsX10,
                                                                       const double &degreeAziumth,  const QDateTime &dtToReckon, bool &isOnLand) const;
    /********************************
     * Assumption:
     *  Each target is installed with all types of positioning devices.
     *  But each data source can only observe part of these devices from part of all targets.
     * ****************************/
    QHash <PB_Enum_TargetInfo_Type, PosDevice*> hashTargetInfoTypePosDevice;

    /*******
     * Periodically update pbTargetPosCurrent, assuming Great Circle journey from pbTargetPosOrig.
     * When the target meets land, change the pbTargetPosOrig,  pbTargetPosCurrent and posTime.
********/
    PBTargetPosition pbTargetPosOrig, pbTargetPosCurrent, pbTargetPosBeforeCurrent;
    QDateTime posOrigDateTime, posCurrentDateTime;
    QGeoCoordinate geoOrigHighPreci,  geoCurrentHighPreci, geoBeforeCurrentHighPreci;

    //PBCoderDecoder *pbCoderDecoder;
    ThreadedWorld *world;
};

#endif // TARGET_H
