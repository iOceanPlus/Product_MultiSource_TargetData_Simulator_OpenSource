#ifndef CONTAINEROFUNTHREADEDMQTOPICPUBLISH_H
#define CONTAINEROFUNTHREADEDMQTOPICPUBLISH_H

#include <QObject>
#include "MQTopicPublishCore.h"
class ContainerOfUnThreadedMQTopicPublish : public QObject
{
    Q_OBJECT
public:
    explicit ContainerOfUnThreadedMQTopicPublish(QString mqAddr, QString exchangeName,
                                                 QObject *parent = 0,quint32 maxCachedMessageCount=300000,quint32 secondsIntervalSendHeartbeatToMQ=5);
    ~ContainerOfUnThreadedMQTopicPublish();

signals:
    void sigErrorInfo(const QString &errorStr) const;
    void sigInfo(const QString &infoStr) const;

public slots:
    //This method can be called directly as well as connect by a signal
    void slotPublishToMQ(const QList <StructDataAndKey> &listDataAndKey) const;

private slots:
    //void slotPublishSimuHeartBeat();

private:
    MQTopicPublishCore *mqTopicPublish;
    QTimer *timerPublishSimuHeartBeatToMQ;
};

#endif // CONTAINEROFUNTHREADEDMQTOPICPUBLISH_H
