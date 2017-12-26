#ifndef TARGET_H
#define TARGET_H

#include <QObject>
#include <QMultiMap>
#include "PBCoderDecoder.h"
#include "ContainerOfThreadMQTopicPublish.h"
#include "PosDevice.h"
#include <QGeoCoordinate>

struct StructKinematicStates
{
    qint64 dateTimeMSecs;
    QGeoCoordinate geoHighPreci;
    double accelSpeedInMeterPerSquareSecond; //Acceleration, m/s^2
    double speedMetersPerSecondCurrentHighPreci;
    double cogInDegreesHighPreci;
};

class ThreadedWorld;

class Target
{
public:
    explicit Target(const PBTargetPosition &pbTargetPosInitial, ThreadedWorld *world, const qint64 &posDateTimeMSecs);
    bool addPosDevice(PB_Enum_TargetInfo_Type infoType, PosDevice* posDev);
    bool installPosDevices(qint64 timeMSecsSinceEpoch);
    std::default_random_engine *getRandEngineOfThisWorld() const;
    ~Target();

    quint64 getTargetIDOrigAggregatedWithIDType(const quint8 &targetID_Type, const quint32 &targetIDOrig);

    /************Update the pbTargetPosOrig when the target meet land **************/
    PBTargetPosition updateTargetAndGetPbTargetPosCurrent(const qint64 &currentDateTimeMSecs);
    PBTargetPosition getPBTargetPosCurrent() const;

    QHash<PB_Enum_TargetInfo_Type, PosDevice *> getHashTargetInfoTypePosDevice() const;
    PosDevice* getDevice(const PB_Enum_TargetInfo_Type &infoType);
    const Struct_PosDeviceInfo getDeviceInfo(const PB_Enum_TargetInfo_Type &infoType) const;

    void setOriginalTargetIDsOfTargetPos(PBTargetPosition &pbTargetPosToSet);
    void  clearInvalidFieldsInAnOriginalTargetPos(PBTargetPosition &pbTargetPos);
    void set_enum_targetidorigAndIDType_AccordingToInfoType(PBTargetPosition &pbTargetPosToSet);

private:
    void updateCurrentPosAndCalibrateCOG(const qint64 &dtMSecsReckoned, const QGeoCoordinate &geoReckoned, const QGeoCoordinate &geoBeforeReckon) ;
    const QGeoCoordinate getConstGeoPosHighPreciReckoned(const QGeoCoordinate &geoOrig, const  qint64 &dtOrigMSecs, const double &avgSpeedInMetersPerSecond,
                                                                       const double &degreeAziumth,  const qint64 &dtToReckonMSecs, bool &isOnLand) const;

    const QGeoCoordinate getConstGeoPosHighPreciReckoned(const QGeoCoordinate &geoOrig, const  double metersTravelled,
                                                                       const double &degreeAziumth,  bool &isOnLand) const;

    void deadReckoning(const qint64 &currentDateTimeMSecs, QGeoCoordinate &geoReckoned, double &newAccelSpeedInMeterPerSquareSecond,
                       double &newSpeedMetersPerSecondCurrentHighPreci, bool &isOutSideAreaFilter) const;
    void calibrateTargetKinematic(const bool &isOutSideArea, const qint64 &dtMSecsReckoned,
                                        const QGeoCoordinate &geoBeforeReckon, const QGeoCoordinate &geoReckoned,
                                        double &newAccelSpeedInMeterPerSquareSecond, double &newSpeedMetersPerSecondCurrentHighPreci);

    void updateSOGAndAcceleration(const qint64 &currentDateTimeMSecs, double &newAccelSpeedInMeterPerSquareSecond,
                             double &newSpeedMetersPerSecondCurrentHighPreci) const;
    void findAndSetNewDirectionIntoWater(const qint64 &currentDateTimeMSecs);
    void detectMovingToBoundaryAndSlowDownWhenClose();
    /********************************
     * Assumption:
     *  Each target is installed with all types of positioning devices.
     *  But each data source can only observe part of these devices from part of all targets.
     * ****************************/
    QHash <PB_Enum_TargetInfo_Type, PosDevice*> hashTargetInfoTypePosDevice;

    /*******
     * Periodically update geoCurrentHighPreci, assuming Great Circle journey from pbTargetPosOrig.
     * When the target meets land, change the geoOrigHighPreci and posTime.
     *
     * geoOrigHighPreci: Start point of this journey.
     * geoCurrentHighPreci: Current point of this journey.
********/
    PBTargetPosition pbTargetPosInitial;
    double initialSpeedInMeterPerSecond;

    StructKinematicStates kinematicOrig, kinematicBeforeCurrent, kinematicCurrent;

    bool stoppedForever;
    ThreadedWorld *world;
};

#endif // TARGET_H
