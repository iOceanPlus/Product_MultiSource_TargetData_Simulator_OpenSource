#ifndef CONTAINTEROFTHREADEDMQTOPICCONSUME_H
#define CONTAINTEROFTHREADEDMQTOPICCONSUME_H

#include <QObject>
class MQTopicConsumeCore;
class QThread;
class QTimer;

class ContainerOfThreadedMQTopicConsume : public QObject
{
    Q_OBJECT
public:
    explicit ContainerOfThreadedMQTopicConsume(QString mqAddr, QString exchangeName, QStringList listRoutingKey, QString consumerTag,
                   QObject *parent = 0, QString queueName="", int messageInQueueTTL_MiliSeconds=300000,
                   int queueTTL_MiliSeconds=600000,
                  QString routingKeyForSimuHeart="SimuHeartBeat", bool isQueueDurable=true, quint32 secondsIntervalSendHeartbeatToMQ=5);
    ~ContainerOfThreadedMQTopicConsume();


signals:
    void sigMsgRcvd(const QByteArray &baData,QString exchangeName,QString routingKey,
                    quint64 deliveryTag, bool redelivered) const;
    void sigErrorInfo(const QString &errorStr) const;
    void sigInfo(const QString &infoStr) const;

private slots:

private:
    MQTopicConsumeCore *mqTopicConsumeCore;
    QThread *threadOfMQTopicConsume;
    QTimer *timerPublishSimuHeartBeatToMQ;
};

#endif // CONTAINTEROFTHREADEDMQTOPICCONSUME_H
