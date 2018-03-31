#ifndef CONTAINEROFUNTHREADEDMQTOPICCONSUME_H
#define CONTAINEROFUNTHREADEDMQTOPICCONSUME_H

#include <QObject>
#include "MQTopicPublishCore.h"
class MQTopicConsumeCore;
class QTimer;

class ContainerOfUnThreadedMQTopicConsume : public QObject
{
    Q_OBJECT
public:
    explicit ContainerOfUnThreadedMQTopicConsume(QString mqAddr, QString exchangeName, QStringList listRoutingKey,QString consumerTag,
                             QObject *parent = 0,QString queueName="",int messageInQueueTTL_MiliSeconds=300000,
                              int queueTTL_MiliSeconds=600000,
                               QString routingKeyForSimuHeart="SimuHeartBeat",bool isQueueDurable=true, quint32 secondsIntervalSendHeartbeatToMQ=5);
    ~ContainerOfUnThreadedMQTopicConsume();

signals:
    void sigMsgRcvd(const QByteArray &baData,QString exchangeName,QString routingKey,
                    quint64 deliveryTag, bool redelivered);
    void sigErrorInfo(const QString &errorStr) const;
    void sigInfo(const QString &infoStr) const;


private slots:
   // void slotPublishSimuHeartBeat();

private:
    MQTopicConsumeCore *mqTopicConsumeCore;
    QTimer *timerPublishSimuHeartBeatToMQ;
};

#endif // CONTAINEROFUNTHREADEDMQTOPICCONSUME_H
