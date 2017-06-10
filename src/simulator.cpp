#include "simulator.h"

Simulator::Simulator(QObject *parent) : QObject(parent)
{
    mutex=new QMutex();
    threadOfWorld=new QThread(this);
    this->thread()->setPriority(QThread::LowPriority);

    world=new World(mutex);
    connect(threadOfWorld,&QThread::finished,world,&QObject::deleteLater);
    world->moveToThread(threadOfWorld);
    threadOfWorld->start(QThread::NormalPriority);

    QStringList listRoutingKeyToConsume;
    listRoutingKeyToConsume.append(ROUTING_KEY_MONITOR_RPOBE);
    ioMessages=new IOMessages(SOFTWARE_NAME,listRoutingKeyToConsume,"param_mq.txt",this);
    connectIOMessageAndWorld();
}

void Simulator::connectIOMessageAndWorld()
{
    connect(ioMessages,SIGNAL(sigPBMonitor(PBMonitor)),world,SIGNAL(sigPBMonitor(PBMonitor)));

    connect(world,SIGNAL(sigSend2MQ(QList<StructDataAndKey>)),
            ioMessages,SLOT(slotPublishToMQ(QList<StructDataAndKey>)));
}

Simulator::~Simulator()
{
    if(threadOfWorld&&threadOfWorld->isRunning())
    {
        threadOfWorld->quit();
        threadOfWorld->wait(500);
        threadOfWorld->deleteLater();
    }
    else if(threadOfWorld)
        threadOfWorld->deleteLater();

    // Optional:  Delete all global objects allocated by libprotobuf.
    google::protobuf::ShutdownProtobufLibrary();
}
