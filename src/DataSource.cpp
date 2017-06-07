#include "DataSource.h"

DataSource::DataSource(ParallelWorld *world, const QMap <PB_Enum_TargetInfo_Type,Struct_TransmissionQuality>  &mapInfoTypeTransmitQuality,
                       QObject *parent ) : QObject(parent)
{
    this->mapInfoTypeTransmitQuality=mapInfoTypeTransmitQuality;
    this->world=world;
}

bool DataSource::fetchDataFromChannelsAndSendToMQ()
{










}
