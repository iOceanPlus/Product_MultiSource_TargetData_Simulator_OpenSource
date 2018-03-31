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
    void sigErrorInfo(const QString errorStr) const;
    void sigInfo(const QString &infoStr) const;

    void sigPublishToMQ(const QList <StructDataAndKey> &listDataAndKey) const;

public slots:
    //This method can be called directly as well as connect by a signal
    void slotPublishToMQ(const QList <StructDataAndKey> &listDataAndKey) const;
private slots:

private:
    MQTopicPublishCore *mqTopicPublish;
    QThread *threadOfMQTopicPublish;
    QTimer *timerPublishSimuHeartBeatToMQ;
};

#endif // CONTAINEROFTHREADMQTOPICPUBLISH_H
