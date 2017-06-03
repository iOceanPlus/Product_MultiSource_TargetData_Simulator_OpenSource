#ifndef PBCODERDECODER_H
#define PBCODERDECODER_H

#include <QObject>
#include "ProtoCPP/Target.pb.h"
#include "ProtoCPP/CommonEnum.pb.h"

class PBCoderDecoder : public QObject
{
    Q_OBJECT
public:
    explicit PBCoderDecoder(PB_Enum_Software enum_SoftwareName,  QObject *parent = 0);
    ~PBCoderDecoder();

    static bool isCoordinatesValid(const PBTargetPosition &targetPos, bool is0Valid=false);


    /************************************************ Serializers *********************************************/
    QByteArray serializePBTargetToArray(PBTarget pbTarget);

    /*********************************************** getters and setters *******************************************/
    quint32 getSerialNumAndIncrement();
    quint32 getSerialNumCommandAndIncrement();
    quint32 getSerialNumConfigAndIncrement();

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
};

#endif // PBCODERDECODER_H
