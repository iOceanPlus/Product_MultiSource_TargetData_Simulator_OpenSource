#ifndef PBCODERDECODER_H
#define PBCODERDECODER_H

#include <QObject>
#include "Target.pb.h"
#include "CommonEnum.pb.h"

class QMutex;

class PBCoderDecoder : public QObject
{
    Q_OBJECT
public:
    explicit PBCoderDecoder(PB_Enum_Software enum_SoftwareName,QMutex *mutex,   QObject *parent = 0);
    ~PBCoderDecoder();

    static bool isCoordinatesValid(const PBTargetPosition &targetPos, bool is0Valid=false);
    static QString getReadableTargetInfo_SourceName(const PB_Enum_TargetInfo_Source &targetInfoSource,const QString &language);
    static QString getReadableTargetInfo_TypeName(const PB_Enum_TargetInfo_Type &targetinfoTypeName, const QString &language);

    static PB_Enum_Aggregated_AIS_Ship_Type getAggregatedAISShipType(const qint16 &shipTypeAIS);
    /************************************************ Serializers *********************************************/
    QByteArray serializePBTargetToArray(PBTarget pbTarget) const;


    /*********************************************** getters and setters *******************************************/
    quint32 getSerialNumAndIncrement();

    PB_Enum_Software getPbEnumSenderSoftware() const;
    void setPbEnumSenderSoftware(const PB_Enum_Software &value);

    quint32 getStartedTimeUTC() const;
    void setStartedTimeUTC(const quint32 &value);

signals:

public slots:

private:
    quint32 serialNum;
    PB_Enum_Software pbEnumSenderSoftware;

    quint32 startedTimeUTC;
    QMutex *mutex;
};

#endif // PBCODERDECODER_H
