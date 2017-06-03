#include "ContainerOfThreadMQTopicPublish.h"
#include <QThread>
#include <QTimer>
#include <QDebug>
#include "MQTopicPublishCore.h"

ContainerOfThreadMQTopicPublish::ContainerOfThreadMQTopicPublish(QString mqAddr, QString exchangeName,
            QObject *parent ,quint32 maxCachedMessageCount,quint32 secondsIntervalSendHeartbeatToMQ) : QObject(parent)
{
    threadOfMQTopicPublish=new QThread(this);
    mqTopicPublish=new MQTopicPublishCore(mqAddr,exchangeName,false, 0,maxCachedMessageCount);
    connect(threadOfMQTopicPublish,SIGNAL(finished()),mqTopicPublish,SLOT(deleteLater()));
    connect(threadOfMQTopicPublish,SIGNAL(started()),mqTopicPublish,SLOT(slotInit()));
    mqTopicPublish->moveToThread(threadOfMQTopicPublish);

    timerPublishSimuHeartBeatToMQ=new QTimer(this);
    timerPublishSimuHeartBeatToMQ->start(secondsIntervalSendHeartbeatToMQ*1000);
    connect(timerPublishSimuHeartBeatToMQ,SIGNAL(timeout()),this,SLOT(slotPublishSimuHeartBeat()));

    threadOfMQTopicPublish->start();
}

void ContainerOfThreadMQTopicPublish::slotPublishToMQ(QList <StructDataAndKey> listDataAndKey)
{
    mqTopicPublish->slotPublish(listDataAndKey);
}

ContainerOfThreadMQTopicPublish::~ContainerOfThreadMQTopicPublish()
{

}

void ContainerOfThreadMQTopicPublish::slotPublishSimuHeartBeat()
{
    mqTopicPublish->slotTimerEventSimuHeartBeat();
}


