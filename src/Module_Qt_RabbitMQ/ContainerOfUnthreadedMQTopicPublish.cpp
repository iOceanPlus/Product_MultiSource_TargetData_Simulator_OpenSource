#include <QTimer>
#include "ContainerOfUnThreadedMQTopicpublish.h"
#include "MQTopicPublishCore.h"
ContainerOfUnThreadedMQTopicPublish::ContainerOfUnThreadedMQTopicPublish(QString mqAddr, QString exchangeName,
         QObject *parent, quint32 maxCachedMessageCount, quint32 secondsIntervalSendHeartbeatToMQ)
        : QObject(parent)
{
    mqTopicPublish=new MQTopicPublishCore(mqAddr,exchangeName,true, this,maxCachedMessageCount);

    timerPublishSimuHeartBeatToMQ=new QTimer(this);
    connect(timerPublishSimuHeartBeatToMQ,SIGNAL(timeout()),mqTopicPublish,SLOT(slotTimerEventSimuHeartBeat()));
    mqTopicPublish->slotInit();
    timerPublishSimuHeartBeatToMQ->start(secondsIntervalSendHeartbeatToMQ*1000);
}

void ContainerOfUnThreadedMQTopicPublish::slotPublishToMQ(QList <StructDataAndKey> listDataAndKey)
{
    mqTopicPublish->slotPublish(listDataAndKey);
}

ContainerOfUnThreadedMQTopicPublish::~ContainerOfUnThreadedMQTopicPublish()
{

}
