#include "target.h"
#include "macro.h"
#include "PBCoderDecoder.h"
#include "ThreadedWorld.h"
#include <QGeoCoordinate>
#include <QDebug>
#include <QtMath>

Target::Target(const PBTargetPosition &pbTargetPos,
               ThreadedWorld *paramWorld, const qint64 &posDateTimeMSecs)
{
    this->hashTargetInfoTypePosDevice=hashTargetInfoTypePosDevice;
    this->world=paramWorld;

    this->pbTargetPosInitial.CopyFrom(pbTargetPos);
    initialSpeedInMeterPerSecond=pbTargetPos.aisdynamic().sogknotsx10()/10.0*KNOT_IN_METER_PER_SECOND;

    kinematicOrig.accelSpeedInMeterPerSquareSecond=0;
    kinematicOrig.cogInDegreesHighPreci=pbTargetPos.aisdynamic().cogdegreex10()/10.0;
    kinematicOrig.dateTimeMSecs=posDateTimeMSecs;
    kinematicOrig.geoHighPreci.setLatitude(pbTargetPos.aisdynamic().intlatitudex60w()/AISPosDivider);
    kinematicOrig.geoHighPreci.setLongitude(pbTargetPos.aisdynamic().intlongitudex60w()/AISPosDivider);
    kinematicOrig.speedMetersPerSecondCurrentHighPreci=pbTargetPos.aisdynamic().sogknotsx10()/10.0*KNOT_IN_METER_PER_SECOND;

    kinematicCurrent=kinematicBeforeCurrent=kinematicOrig;
    stoppedForever=pbTargetPos.aisdynamic().sogknotsx10()<=0;
}

Target::~Target()
{
    QHashIterator <PB_Enum_TargetInfo_Type, PosDevice*> iHashTargetInfoTypePosDevice(hashTargetInfoTypePosDevice);
    while(iHashTargetInfoTypePosDevice.hasNext())
    {
        iHashTargetInfoTypePosDevice.next();
        delete iHashTargetInfoTypePosDevice.value();
    }
}

bool Target::installPosDevices(qint64 timeMSecsSinceEpoch)
{
    QMapIterator <PB_Enum_TargetInfo_Type, Struct_PosDeviceInfo> iMapInfoTypePosDeviceInfo(world->getMapInfoTypePosDeviceInfo());
    while(iMapInfoTypePosDeviceInfo.hasNext())
    {
        iMapInfoTypePosDeviceInfo.next();
        PB_Enum_TargetInfo_Type infoType= iMapInfoTypePosDeviceInfo.key();
        Struct_PosDeviceInfo posDevInfo=iMapInfoTypePosDeviceInfo.value();
        if(hashTargetInfoTypePosDevice.contains(infoType))
        {
            qDebug()<<"Warning: try to install already existed devices. Ignored.";
            continue;
        }
        PosDevice *posDev=new PosDevice(timeMSecsSinceEpoch-qrand()%posDevInfo.sampleMilliSeconds,this,infoType);
        hashTargetInfoTypePosDevice.insert(infoType,posDev);
    }
    return true;
}

PBTargetPosition Target::updateTargetAndGetPbTargetPosCurrent(const qint64 &currentDateTimeMSecs)
{
    if((currentDateTimeMSecs-kinematicCurrent.dateTimeMSecs)<ExternV_MIN_Sample_MSEC
            ||stoppedForever)
        return getPBTargetPosCurrent();

    StructKinematicStates lastCurrentKinematic=kinematicCurrent; //Store it first, kinematicCurrent will be modified
    QGeoCoordinate geoReckoned ;
    double newAccelSpeedInMeterPerSquareSecond,newSpeedMetersPerSecondCurrentHighPreci;
    bool isOutSideAreaFilter;
    deadReckoning(currentDateTimeMSecs,geoReckoned,newAccelSpeedInMeterPerSquareSecond,newSpeedMetersPerSecondCurrentHighPreci,
                  isOutSideAreaFilter);
    calibrateTargetKinematic(isOutSideAreaFilter,currentDateTimeMSecs,lastCurrentKinematic.geoHighPreci, geoReckoned,
                                   newAccelSpeedInMeterPerSquareSecond,newSpeedMetersPerSecondCurrentHighPreci);
    kinematicBeforeCurrent=lastCurrentKinematic;
#ifdef DEBUG_MOTION
    std::cout<<getPBTargetPosCurrent().DebugString();
#endif
    return getPBTargetPosCurrent();
}

void Target::deadReckoning(const qint64 &currentDateTimeMSecs, QGeoCoordinate &geoReckoned ,
                           double &newAccelSpeedInMeterPerSquareSecond,double &newSpeedMetersPerSecondCurrentHighPreci,
                           bool &isOutSideAreaFilter) const
{
    updateSOGAndAcceleration(currentDateTimeMSecs,newAccelSpeedInMeterPerSquareSecond,newSpeedMetersPerSecondCurrentHighPreci);
    geoReckoned= getConstGeoPosHighPreciReckoned(kinematicOrig.geoHighPreci,kinematicOrig.dateTimeMSecs,
                                ( kinematicCurrent.speedMetersPerSecondCurrentHighPreci+newSpeedMetersPerSecondCurrentHighPreci)/2.0,
                                kinematicOrig.cogInDegreesHighPreci, currentDateTimeMSecs, isOutSideAreaFilter);

}

void Target::calibrateTargetKinematic(const bool &isOutSideArea,  const qint64 &dtMSecsReckoned,
                            const  QGeoCoordinate &geoBeforeReckon, const  QGeoCoordinate &geoReckoned,
                                            double &newAccelSpeedInMeterPerSquareSecond,double &newSpeedMetersPerSecondCurrentHighPreci)
{
    if(!isOutSideArea) //inside area filter
    {
        updateCurrentPosAndCalibrateCOG(dtMSecsReckoned,geoReckoned,geoBeforeReckon);
        kinematicCurrent.accelSpeedInMeterPerSquareSecond=newAccelSpeedInMeterPerSquareSecond;
        kinematicCurrent.speedMetersPerSecondCurrentHighPreci=newSpeedMetersPerSecondCurrentHighPreci;
        detectMovingToBoundaryAndSlowDownWhenClose();
    }
    else  //outside area filter
    {
        findAndSetNewDirectionIntoWater(dtMSecsReckoned);
        if(!stoppedForever)
        {
            detectMovingToBoundaryAndSlowDownWhenClose();
        }
    }
}

void Target::findAndSetNewDirectionIntoWater(const qint64 &currentDateTimeMSecs)
{
    bool turnRight=qrand()%2==0;
    bool newCOGGot=false;
    for(int i=1;i<=7;i++)
    {
        qint32 factor=turnRight?1:-1;
        qint32 newCOGX10=qRound(kinematicCurrent.cogInDegreesHighPreci*10)+(qint32)factor*i*DegreesX10_ToTurn_WhenMeetLand;
        if(newCOGX10<0)
            newCOGX10+=3600;
        else
            newCOGX10%=3600;

        bool isNewPosOnLand;
        QGeoCoordinate geoReckonedTrial= getConstGeoPosHighPreciReckoned(kinematicCurrent.geoHighPreci,kinematicCurrent.dateTimeMSecs,
                                                         kinematicCurrent.speedMetersPerSecondCurrentHighPreci,
                                                                       newCOGX10/10.0, currentDateTimeMSecs, isNewPosOnLand);
        if(isNewPosOnLand)
            continue;
        else
        {
            newCOGGot=true;
            kinematicCurrent.cogInDegreesHighPreci=newCOGX10/10.0;
            kinematicCurrent.accelSpeedInMeterPerSquareSecond=ACCEL_IN_METERS_PER_SECOND; //speed up
            kinematicCurrent.geoHighPreci=geoReckonedTrial;
            kinematicCurrent.dateTimeMSecs=currentDateTimeMSecs;

            kinematicOrig=kinematicCurrent;
            break;
        }
    }
    if(!newCOGGot) //stop it
    {
        kinematicCurrent.speedMetersPerSecondCurrentHighPreci=0;
        kinematicCurrent.dateTimeMSecs=qRound(currentDateTimeMSecs/1000.0);
        stoppedForever=true;
    }
}

void Target::detectMovingToBoundaryAndSlowDownWhenClose()
{
    if(kinematicCurrent.accelSpeedInMeterPerSquareSecond<=(-1*ACCEL_IN_METERS_PER_SECOND)||
            kinematicCurrent.speedMetersPerSecondCurrentHighPreci<=APPROACHING_SPEED_IN_METERS_PER_SECOND)
        return;

    double secondsToApproachSpeed=(kinematicCurrent.speedMetersPerSecondCurrentHighPreci-
                                   APPROACHING_SPEED_IN_METERS_PER_SECOND)/ACCEL_IN_METERS_PER_SECOND;
    double distanceToApproachInMeters=(kinematicCurrent.speedMetersPerSecondCurrentHighPreci+APPROACHING_SPEED_IN_METERS_PER_SECOND)
                                       *secondsToApproachSpeed/2.0;
    bool outsideAreaFilter;
    const double ENLARGE_FACTOR=1.01; //Stop before exactly outside area filter
    getConstGeoPosHighPreciReckoned(kinematicCurrent.geoHighPreci,distanceToApproachInMeters*ENLARGE_FACTOR,
                                    kinematicCurrent.cogInDegreesHighPreci,outsideAreaFilter);
    if(outsideAreaFilter)
        kinematicCurrent.accelSpeedInMeterPerSquareSecond=-1*ACCEL_IN_METERS_PER_SECOND; //decrease speed to sop
}

void Target::updateCurrentPosAndCalibrateCOG(const qint64 &dtMSecsReckoned, const QGeoCoordinate &geoReckoned,
                                             const QGeoCoordinate &geoBeforeReckon)
{
    if(geoBeforeReckon!=geoReckoned)
    {
        float newCOGInDegree=geoBeforeReckon.azimuthTo(geoReckoned);
        kinematicCurrent.cogInDegreesHighPreci=newCOGInDegree;
    }
    kinematicCurrent.geoHighPreci=geoReckoned;
    kinematicCurrent.dateTimeMSecs=dtMSecsReckoned;
}

void Target::updateSOGAndAcceleration(const qint64 &currentDateTimeMSecs, double &newAccelSpeedInMeterPerSquareSecond,
                                      double &newSpeedMetersPerSecondCurrentHighPreci) const
{
    if(stoppedForever)
        return;

    double secondsElapsed=(currentDateTimeMSecs-kinematicCurrent.dateTimeMSecs)/1000.0;
    if(secondsElapsed<=0)
    {
        newAccelSpeedInMeterPerSquareSecond=kinematicCurrent.accelSpeedInMeterPerSquareSecond;
        newSpeedMetersPerSecondCurrentHighPreci=kinematicCurrent.speedMetersPerSecondCurrentHighPreci;
        return;
    }

    if(kinematicCurrent.accelSpeedInMeterPerSquareSecond!=0)  //In accelerating state
    {
        double tentativeSpeed= kinematicCurrent.speedMetersPerSecondCurrentHighPreci+secondsElapsed*kinematicCurrent.accelSpeedInMeterPerSquareSecond;
        if(kinematicCurrent.accelSpeedInMeterPerSquareSecond>0) //speeding up
        {
            if(tentativeSpeed>=initialSpeedInMeterPerSecond) //Already at original speed
            {
                newSpeedMetersPerSecondCurrentHighPreci=initialSpeedInMeterPerSecond;
                newAccelSpeedInMeterPerSquareSecond=0; //Enter into constant speed move
            }
        }
        else if(kinematicCurrent.accelSpeedInMeterPerSquareSecond<0) //slowing down
        {
            if(tentativeSpeed<=APPROACHING_SPEED_IN_METERS_PER_SECOND)
            {
                newSpeedMetersPerSecondCurrentHighPreci=APPROACHING_SPEED_IN_METERS_PER_SECOND;
                newAccelSpeedInMeterPerSquareSecond=0;
            }
        }
        else //continue accelerating
        {
            newSpeedMetersPerSecondCurrentHighPreci=tentativeSpeed;
        }
    }
    else //not accelerating
    {
        newAccelSpeedInMeterPerSquareSecond=kinematicCurrent.accelSpeedInMeterPerSquareSecond;
        newSpeedMetersPerSecondCurrentHighPreci=kinematicCurrent.speedMetersPerSecondCurrentHighPreci;
    }
}
