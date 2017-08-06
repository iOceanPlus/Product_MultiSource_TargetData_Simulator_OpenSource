#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <QObject>
#include <QMutex>
#include <QThread>
#include "IOMessages.h"
#include "ThreadedWorld.h"

class MyQtGeoPolygon;
class Simulator : public QObject
{
    Q_OBJECT
public:
    explicit Simulator(QObject *parent = 0);
    ~Simulator();
    static bool checkJsonObjectAndOutPutValue(const QJsonObject &jsonObject,  const QString &key, const bool &outPutValue);

signals:

public slots:

private:
    void initWaterGrids();
    void initTargetsAndPutToWorlds();

    bool setWorldThreadCountAndBoundingRegionFromParamJson();
    void parseParamFileAndInitWorldMembers();

    void connectIOMessageAndWorld();

    IOMessages *ioMessages;
    QList <ThreadedWorld *> listOfThreadedWorlds;
    qint16 worldThreadCount;
    QMutex *mutex;

    QList <QThread *> listOfWorldThreads;

    MyQtGeoPolygon *sharedGeoPolyGonBoundingRegion;
    QString waterGridsFileName,ship_FileName, mc2FileName, language;

    QSet <qint32> setDistinctTargetIDOrig;
    QMap <PB_Enum_TargetInfo_Type, QSet <qint32> > mapInfoTypeSetTargetID;

    PBCoderDecoder *pbCoderDecoder;  //*pbCoderDecoder
};

#endif // SIMULATOR_H
