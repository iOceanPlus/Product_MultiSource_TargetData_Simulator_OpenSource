#include <QCoreApplication>
#include "Target.pb.h"
#include "Monitor.pb.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    qRegisterMetaType<PBTarget>("PBTarget");
    qRegisterMetaType<PBMonitor>("PBMonitor");
    return a.exec();
}
