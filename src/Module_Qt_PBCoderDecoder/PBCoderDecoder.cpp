#include "PBCoderDecoder.h"
#include <QDebug>
#include <QDateTime>

//#include "macro.h"
PBCoderDecoder::PBCoderDecoder(PB_Enum_Software enum_SoftwareName, QObject *parent) : QObject(parent)
{
    serialNum=0;
    this->pbEnumSenderSoftware = enum_SoftwareName;

    startedTimeUTC=QDateTime::currentDateTime().toTime_t();
    // Verify that the version of the library that we linked against is
    // compatible with the version of the headers we compiled against.
    GOOGLE_PROTOBUF_VERIFY_VERSION;
}

PBCoderDecoder::~PBCoderDecoder()
{

}


quint32 PBCoderDecoder::getStartedTimeUTC() const
{
    return startedTimeUTC;
}

void PBCoderDecoder::setStartedTimeUTC(const quint32 &value)
{
    startedTimeUTC = value;
}

PB_Enum_Software PBCoderDecoder::getPbEnumSenderSoftware() const
{
    return pbEnumSenderSoftware;
}

void PBCoderDecoder::setPbEnumSenderSoftware(const PB_Enum_Software &value)
{
    pbEnumSenderSoftware = value;
}

quint32 PBCoderDecoder::getSerialNumAndIncrement()
{
    quint32 result=serialNum;
    serialNum++;
    return result;
}

QByteArray PBCoderDecoder::serializePBTargetToArray(PBTarget pbTarget)
{
    QByteArray baResult;
    baResult.resize(pbTarget.ByteSize());
    pbTarget.SerializeToArray(baResult.data(),pbTarget.ByteSize());
    return baResult;
}

