#include "DataSource.h"

DataSource::DataSource(ParallelWorld *world, const QMap <PB_Enum_TargetInfo_Type,Struct_TransmitQuality>  &mapInfoTypeTransmitQuality,
                       QObject *parent ) : QObject(parent)
{
    this->mapInfoTypeTransmitQuality=mapInfoTypeTransmitQuality;
    this->world=world;
}

QList <StructDataAndKey> DataSource::fetchDataFromChannels()
{










}
