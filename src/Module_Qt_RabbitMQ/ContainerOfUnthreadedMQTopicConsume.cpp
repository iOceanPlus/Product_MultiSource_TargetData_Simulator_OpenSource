#include <QTimer>
#include "ContainerOfUntTreadedMQTopicConsume.h"
#include "MQTopicConsumeCore.h"
ContainerOfUnThreadedMQTopicConsume::ContainerOfUnThreadedMQTopicConsume(QString mqAddr, QString exchangeName, QStringList listRoutingKey, QString consumerTag,
                QObject *parent, QString queueName, int messageInQueueTTL_MiliSeconds, int queueTTL_MiliSeconds,
                  QString routingKeyForSimuHeart, bool isQueueDurable,   quint32 secondsIntervalSendHeartbeatToMQ) : QObject(parent)
{
    mqTopicConsumeCore=new MQTopicConsumeCore(mqAddr,exchangeName,listRoutingKey,consumerTag,true, this,queueName,
                     messageInQueueTTL_MiliSeconds,queueTTL_MiliSeconds, routingKeyForSimuHeart,isQueueDurable);
    timerPublishSimuHeartBeatToMQ=new QTimer(this);
    connect(timerPublishSimuHeartBeatToMQ,SIGNAL(timeout()),mqTopicConsumeCore,SLOT(slotPublishSimuHeartBeat()));

    connect(mqTopicConsumeCore,SIGNAL(sigMsgRcvd(QByteArray,QString,QString,quint64,bool)),
            this,SIGNAL(sigMsgRcvd(QByteArray,QString,QString,quint64,bool)));
    mqTopicConsumeCore->slotInit();
    timerPublishSimuHeartBeatToMQ->start(secondsIntervalSendHeartbeatToMQ*1000);
}

ContainerOfUnThreadedMQTopicConsume::~ContainerOfUnThreadedMQTopicConsume()
{

}
