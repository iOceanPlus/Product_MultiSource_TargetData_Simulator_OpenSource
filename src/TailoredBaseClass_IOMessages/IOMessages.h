#ifndef IOMESSAGES_H
#define IOMESSAGES_H

#include <QObject>
#include <QStringList>
#include "ContainerOfUnThreadedMQTopicpublish.h"
#include "Target.pb.h"
#include "Monitor.pb.h"
#include "ContainerOfThreadMQTopicPublish.h"
#include "ContainerOfThreadedMQTopicConsume.h"

/********************
 * routing keys of rabbitmq
 * ***************************/
const QString ROUTING_KEY_MONITOR_PROBEACK ="Monitor.ProbeAck";
const QString ROUTING_KEY_MONITOR_ALIVETARGETCOUNT ="Monitor.AliveTargetCount";

const QString ROUTING_KEY_ONLINE_PREPROCESSED_AIS ="OnLine.PreprocessedData.AIS";
const QString ROUTING_KEY_ONLINE_PREPROCESSED_Beidou ="OnLine.PreprocessedData.BeiDou";
const QString ROUTING_KEY_ONLINE_PREPROCESSED_LRIT ="OnLine.PreprocessedData.LRIT";
const QString ROUTING_KEY_ONLINE_PREPROCESSED_Haijian ="OnLine.PreprocessedData.HaiJian";
const QString ROUTING_KEY_ONLINE_PREPROCESSED_Argos ="OnLine.PreprocessedData.Argos";

class QMutex;
class ContainerOfUnThreadedMQTopicConsume;
struct StructMQParams
{
    QString mqAddr,exchangeNameIn,exchangeNameOut, mqQueueName,consumerTag;
    QStringList listRoutingKeyToConsume;
};

class IOMessages: public QObject
{
    Q_OBJECT
public:
    explicit IOMessages(const PB_Enum_Software &enum_SoftwareName, const QStringList &listRoutingKeyToConsume,
                        const QString &mqParamFileName="param_mq.txt", const bool &threaded=false, QObject *parent = 0);
    explicit IOMessages(const PB_Enum_Software &enum_SoftwareName , const StructMQParams &structMQParamsPassedIn,
                        const bool &threaded=false, QObject *parent = 0);
    ~IOMessages();
signals:   
    void sigPBTarget(const PBTarget &pbTarget) const;
    void sigPBMonitor(const PBMonitor &pbMonitor) const; //

    void sigErrorInfo(const QString &errorStr) const;
    void sigInfo(const QString &infoStr) const;

    void sigPublishToMQ(const QList <StructDataAndKey> &listDataAndKey) const; //emit when isContainerThreaded

private slots:
    virtual void slotMsgRcvdFromMQ(const QByteArray &baData, const QString &exchangeName, const QString &routingKey,
                   const quint64 &deliveryTag, const bool &redelivered) ;

public slots:
    //This method can be called directly as well as connect by a signal
    void slotPublishToMQ(const QList <StructDataAndKey> &listDataAndKey) const;
private:
    virtual bool parseMQParamFile(QString mqParamFIleName);
    void startWork();

    QMutex *mutex;

    QString mqIP,exchangeNameIn,exchangeNameOut,  mqQueueName,consumerTag;
    bool outPutMsg, outPutTargetPosInMsg,debugTimeStamp;//For debug

    ContainerOfUnThreadedMQTopicPublish *containerUnThreadedMQTopicPublish;
    ContainerOfUnThreadedMQTopicConsume *containerUnThreadedMQTopicConsume;
    ContainerOfThreadMQTopicPublish *containerThreadedMQTopicPublish;
    ContainerOfThreadedMQTopicConsume *containerThreadedMQTopicConsume;


    StructMQParams structMQParams;

   // PBMonitor_ProbeAck monitorProbeAck;
    PB_Enum_Software enumSoftware;

    bool isContainerThreaded;
};

#endif // IOMESSAGES_H
