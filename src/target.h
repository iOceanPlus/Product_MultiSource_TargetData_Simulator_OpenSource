#ifndef TARGET_H
#define TARGET_H

#include <QObject>
#include <QMultiMap>
#include "PBCoderDecoder.h"
#include "ContainerOfThreadMQTopicPublish.h"

class ParallelWorld;

class Target
{
public:
    explicit Target(PBTargetPosition pbTargetPos, ParallelWorld *parallelWorld, qint32 secondsWindowSizeOfTargetPos);
    ~Target();

public:
    //获得deltaSeconds后的船位置等
    static PBAISDynamic getReckonedAISDynamic(const PBAISDynamic &pbAISDynamic,
                                              const qint32 &deltaSecondsLater, double &stdDevDistance);
    const PBTargetPosition& getConstPbTargetPosFusedLast() const;
    PBTargetPosition &getPbTargetPosFusedLastRef();
    quint64 getTargetIDOrigAggregatedWithIDType(const quint8 &targetID_Type, const quint32 &targetIDOrig);

private:

    PBTargetPosition pbTargetPosFusedLast;
    PBCoderDecoder *pbCoderDecoder;
    ParallelWorld *parallelWorld;

};

#endif // TARGET_H
