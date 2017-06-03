#ifndef CONTAINEROFTHREADMQTOPICPUBLISH_H
#define CONTAINEROFTHREADMQTOPICPUBLISH_H

#include <QObject>
#include "MQTopicPublishCore.h"
class MQTopicPublishCore;
class QThread;
class QTimer;

class ContainerOfThreadMQTopicPublish : public QObject
{
    Q_OBJECT
public:
    explicit ContainerOfThreadMQTopicPublish(QString mqAddr, QString exchangeName,
                         QObject *parent = 0,quint32 maxCachedMessageCount=300000,quint32 secondsIntervalSendHeartbeatToMQ=5);
    ~ContainerOfThreadMQTopicPublish();


signals:

public slots:
    //This method can be called directly as well as connect by a signal
    void slotPublishToMQ(QList <StructDataAndKey> listDataAndKey);

private slots:
    void slotPublishSimuHeartBeat();

private:
    MQTopicPublishCore *mqTopicPublish;
    QThread *threadOfMQTopicPublish;
    QTimer *timerPublishSimuHeartBeatToMQ;
};

#endif // CONTAINEROFTHREADMQTOPICPUBLISH_H
