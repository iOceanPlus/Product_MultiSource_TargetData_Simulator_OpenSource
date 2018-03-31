#ifndef MQTOPICCONSUME_H
#define MQTOPICCONSUME_H

#include "amqpcpp.h"
#include <QObject>
#include <QStringList>
#include <QMutex>
#include "ev.h"

#define RECONNECT_INTERVAL_SECONDS 30

class LukeEventHandler;
class QTimer;
class MQTopicConsumeCore : public QObject
{
    Q_OBJECT
public:
    // If queueName is empty, a random queueName will get from rabbitmq
    explicit MQTopicConsumeCore(QString mqAddr, QString paramExchangeName, QStringList paramListRoutingKey,QString consumerTag,bool evRunNoWait=true,
                    QObject *parent = 0,QString paramQueueName="", int paramMessageInQueueTTL_MiliSeconds=300000,int paramQueueTTL_MiliSeconds=600000,
                    QString paramRoutingKeyForSimuHeart="SimuHeartBeat",bool paramIsQueueDurable=true);
    ~MQTopicConsumeCore();

signals:
    void sigMsgRcvd(const QByteArray &baData,QString exchangeName,QString routingKey,
                    quint64 deliveryTag, bool redelivered) const;
    void sigErrorInfo(const QString &errorStr) const;
    void sigInfo(const QString &infoStr) const;

public slots:
    void slotPublishSimuHeartBeat();
    void slotInit();

private slots:
   void slotTimerEventRunEv();

    void slotDestroyAndReInitAfterAWhile();
    void slotDestroyAndReInitImmediately();

private:
    bool isQueueDurable;
    bool isChannelReady;
    bool evRunNoWait;

    QTimer *timerRunEv;

    //This must be the type int or qint32, or else will get the error "connection reset by peer"
    int messageInQueueTTL_MiliSeconds,queueTTL_Miliseconds;
    QString routingKeyForSimuHeart;
    QString mqAddr,exchangeName,queueName,consumerTag;
    QStringList listRoutingKey;

    LukeEventHandler *eventHandler;
    AMQP::TcpConnection *connection;
    AMQP::TcpChannel *channel;
    QMutex *mutex;

    void  connectMQAndDeclare();
};

#endif // MQTOPICCONSUME_H
