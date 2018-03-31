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
    connect(mqTopicPublish,SIGNAL(sigErrorInfo(QString)),this,SIGNAL(sigErrorInfo(QString)));
    connect(mqTopicPublish, SIGNAL(sigInfo(QString)),this,SIGNAL(sigInfo(QString)));

    mqTopicPublish->slotInit();
    timerPublishSimuHeartBeatToMQ->start(secondsIntervalSendHeartbeatToMQ*1000);
}

void ContainerOfUnThreadedMQTopicPublish::slotPublishToMQ(const QList<StructDataAndKey> &listDataAndKey) const
{
    mqTopicPublish->slotPublish(listDataAndKey);
}

ContainerOfUnThreadedMQTopicPublish::~ContainerOfUnThreadedMQTopicPublish()
{

}
