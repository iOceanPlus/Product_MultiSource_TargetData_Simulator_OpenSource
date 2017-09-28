#include <QCoreApplication>
#include "Target.pb.h"
#include "Monitor.pb.h"
#include "simulator.h"
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    qRegisterMetaType<PBTarget>("PBTarget");
    qRegisterMetaType<PBMonitor>("PBMonitor");
    qRegisterMetaType < QList<StructDataAndKey> >("QList<StructDataAndKey>");

    qRegisterMetaType<PB_Enum_TargetInfo_Source>("PB_Enum_TargetInfo_Source");
    qRegisterMetaType<QMap <PB_Enum_TargetInfo_Type,Struct_TransmissionQuality> >("QMap <PB_Enum_TargetInfo_Type,Struct_TransmissionQuality>");
    qRegisterMetaType<Struct_TransmissionQuality>("Struct_TransmissionQuality");
    qRegisterMetaType<QList<PBTargetPosition> >("QList<PBTargetPosition>");
    qRegisterMetaType<QMap<PB_Enum_TargetInfo_Type, Struct_PosDeviceInfo> >("QMap<PB_Enum_TargetInfo_Type, Struct_PosDeviceInfo>");

    qDebug()<<"Update date: "<<"20170928";

    // Verify that the version of the library that we linked against is
    // compatible with the version of the headers we compiled against.
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    Simulator simu;
    return a.exec();
}
