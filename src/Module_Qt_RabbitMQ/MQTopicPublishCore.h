#ifndef MQTOPICPUBLISH_H
#define MQTOPICPUBLISH_H

#include "amqpcpp.h"
#include <QObject>
#include <QMutex>
#include "MQTopicConsumeCore.h"
#include "ev.h"

struct StructDataAndKey
{
    QByteArray data;
    QString routingKey;
};
class LukeEventHandler;
class QTimer;

class MQTopicPublishCore : public QObject
{
    Q_OBJECT
public:
    //MQTopicPublish uses a list as a cache for messages to be published. When the size is more than maxChachedMessageCount, remove
    //oldest ones.
    explicit MQTopicPublishCore(QString mqAddr, QString exchangeName, bool evRunNoWait=true,
                                QObject *parent = 0,quint32 maxCachedMessageCount=300000);
    ~MQTopicPublishCore();

signals:
    void sigErrorInfo(const QString errorStr) const;
    void sigInfo(const QString &infoStr) const;
public slots:
    void slotPublish(const QList<StructDataAndKey> listDataAndKey);
    void slotInit();
    void slotTimerEventSimuHeartBeat();

private slots:

    void slotDestroyAndReInitAfterAWhile();
    void slotDestroyAndReInitImmediately();

    void slotTimerEventRunEv();

private:
    bool isChannelReady, evRunNoWait;
    QList <StructDataAndKey> listDataAndKeyToBePublished;

    LukeEventHandler *eventHandler;
    AMQP::TcpConnection *connection;
    AMQP::TcpChannel *channel;

    QString exchangeName;
    QString mqAddr;
    quint32 maxCachedMessageCount;
    QTimer *timerRunEv;
    QMutex *mutex;

    void  connectMQAndDeclare();
};

#endif // MQTOPICPUBLISH_H
