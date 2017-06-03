#include "ContainerOfThreadedMQTopicConsume.h"
#include <QThread>
#include <QTimer>
#include "MQTopicConsumeCore.h"

ContainerOfThreadedMQTopicConsume::ContainerOfThreadedMQTopicConsume(QString mqAddr, QString exchangeName,
              QStringList listRoutingKey, QString consumerTag, QObject *parent, QString queueName, int messageInQueueTTL_MiliSeconds, int queueTTL_MiliSeconds,
              QString routingKeyForSimuHeart, bool isQueueDurable, quint32 secondsIntervalSendHeartbeatToMQ) : QObject(parent)
{
    threadOfMQTopicConsume=new QThread(this);
    mqTopicConsumeCore=new MQTopicConsumeCore(mqAddr,exchangeName,listRoutingKey,consumerTag,false, 0,queueName,
                                         messageInQueueTTL_MiliSeconds,  queueTTL_MiliSeconds,
                                    routingKeyForSimuHeart,isQueueDurable);
    connect(threadOfMQTopicConsume,SIGNAL(finished()),mqTopicConsumeCore,SLOT(deleteLater()));
    connect(threadOfMQTopicConsume,SIGNAL(started()),mqTopicConsumeCore,SLOT(slotInit()));
    mqTopicConsumeCore->moveToThread(threadOfMQTopicConsume);

    timerPublishSimuHeartBeatToMQ=new QTimer(this);
    timerPublishSimuHeartBeatToMQ->start(secondsIntervalSendHeartbeatToMQ*1000);
    connect(timerPublishSimuHeartBeatToMQ,SIGNAL(timeout()),this,SLOT(slotPublishSimuHeartBeat()));

    connect(mqTopicConsumeCore,SIGNAL(sigMsgRcvd(QByteArray,QString,QString,quint64,bool)),
            this,SIGNAL(sigMsgRcvd(QByteArray,QString,QString,quint64,bool)));

    threadOfMQTopicConsume->start();
}

ContainerOfThreadedMQTopicConsume::~ContainerOfThreadedMQTopicConsume()
{

}

//avoid reset caused by missing heartbeat
void ContainerOfThreadedMQTopicConsume::slotPublishSimuHeartBeat()
{
    mqTopicConsumeCore->slotPublishSimuHeartBeat();
}


