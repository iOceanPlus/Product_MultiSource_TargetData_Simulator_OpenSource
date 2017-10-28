#ifndef IOMESSAGES_H
#define IOMESSAGES_H

#include <QObject>
#include <QStringList>
#include "ProtoCPP/Target.pb.h"
#include "ProtoCPP/Monitor.pb.h"
#include "ContainerOfUnThreadedMQTopicpublish.h"

/********************
 * routing keys of rabbitmq
 * ***************************/
const QString ROUTING_KEY_MONITOR_PROBEACK ="Monitor.ProbeAck";
const QString ROUTING_KEY_MONITOR_ALIVETARGETCOUNT ="Monitor.AliveTargetCount";
const QString ROUTING_KEY_FUSEDDATA_TARGETTRACKS = "OnLine.FusedData.TargetTracks";

const QString ROUTING_KEY_ONLINE_PREPROCESSED_AIS ="OnLine.PreprocessedData.AIS";
const QString ROUTING_KEY_ONLINE_PREPROCESSED_Beidou ="OnLine.PreprocessedData.BeiDou";
const QString ROUTING_KEY_ONLINE_PREPROCESSED_LRIT ="OnLine.PreprocessedData.LRIT";
const QString ROUTING_KEY_ONLINE_PREPROCESSED_Haijian ="OnLine.PreprocessedData.HaiJian";
const QString ROUTING_KEY_ONLINE_PREPROCESSED_Argos ="OnLine.PreprocessedData.Argos";


class QMutex;
class ContainerOfUnThreadedMQTopicConsume;

class IOMessages: public QObject
{
    Q_OBJECT
public:
    explicit IOMessages(const PB_Enum_Software &enum_SoftwareName , QStringList paramListRoutingKeyToConsume,
                        QString mqParamFileName="param_mq.txt", QObject *parent = 0);
    ~IOMessages();
signals:
    void sigPBTarget(PBTarget pbTarget);
    void sigPBMonitor(PBMonitor pbMonitor); //

private slots:
    virtual void slotMsgRcvdFromMQ(QByteArray baData, QString exchangeName, QString routingKey,
                    quint64 deliveryTag, bool redelivered);

public slots:
    //This method can be called directly as well as connect by a signal
    void slotPublishToMQ(QList <StructDataAndKey> listDataAndKey);
private:
    virtual bool parseMQParamFile(QString mqParamFIleName);
    void startWork();

    QMutex *mutex;

    QString mqIP,exchangeNameIn,exchangeNameOut,  mqQueueName,consumerTag;

    ContainerOfUnThreadedMQTopicPublish *containerUnThreadedMQTopicPublish;
    ContainerOfUnThreadedMQTopicConsume *containerUnThreadedMQTopicConsume;
    QStringList listRoutingKeyToConsume;

   // PBMonitor_ProbeAck monitorProbeAck;
    PB_Enum_Software enumSoftware;
};

#endif // IOMESSAGES_H
