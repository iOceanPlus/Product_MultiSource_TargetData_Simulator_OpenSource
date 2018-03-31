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
    connect(mqTopicPublish,SIGNAL(sigErrorInfo(QString)),this,SIGNAL(sigErrorInfo(QString)));
    connect(mqTopicPublish,SIGNAL(sigInfo(QString)),this,SIGNAL(sigInfo(QString)));

    mqTopicPublish->moveToThread(threadOfMQTopicPublish);

    timerPublishSimuHeartBeatToMQ=new QTimer(this);
    timerPublishSimuHeartBeatToMQ->start(secondsIntervalSendHeartbeatToMQ*1000);
    connect(timerPublishSimuHeartBeatToMQ,SIGNAL(timeout()),this,SLOT(slotPublishSimuHeartBeat()));//Can not connect with mqTopicPublish
  //  connect(this,SIGNAL(sigPublishToMQ(QList<StructDataAndKey>)),mqTopicPublish,SLOT(slotPublish(QList<StructDataAndKey>))); //This fail to funct

    threadOfMQTopicPublish->start();
}


ContainerOfThreadMQTopicPublish::~ContainerOfThreadMQTopicPublish()
{
    threadOfMQTopicPublish->quit();
    threadOfMQTopicPublish->wait(1000);
    threadOfMQTopicPublish->deleteLater();
}

void ContainerOfThreadMQTopicPublish::slotPublishToMQ(const QList<StructDataAndKey> &listDataAndKey) const
{
    mqTopicPublish->slotPublish(listDataAndKey);
}

void ContainerOfThreadMQTopicPublish::slotPublishSimuHeartBeat()
{
    mqTopicPublish->slotTimerEventSimuHeartBeat();
}
