#include <QCoreApplication>
#include "Target.pb.h"
#include "Monitor.pb.h"
#include "simulator.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    qRegisterMetaType<PBTarget>("PBTarget");
    qRegisterMetaType<PBMonitor>("PBMonitor");
    qRegisterMetaType < QList<StructDataAndKey> >("QList<StructDataAndKey>");

    // Verify that the version of the library that we linked against is
    // compatible with the version of the headers we compiled against.
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    Simulator simu;
    return a.exec();
}
