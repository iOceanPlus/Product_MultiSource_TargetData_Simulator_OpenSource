#include <QMutex>
#include <QDateTime>
#include <QFile>
#include <QDebug>
#include "ContainerOfUnThreadedMQTopicpublish.h"
#include "ContainerOfUntTreadedMQTopicConsume.h"
#include "IOMessages.h"

#include <exception>
using namespace std;

IOMessages::IOMessages(const PB_Enum_Software &enum_SoftwareName, QStringList paramListRoutingKeyToConsume,
                       QString mqParamFileName, QObject *parent) :
    QObject(parent)
{
    mutex=new QMutex();
    containerUnThreadedMQTopicPublish = NULL;
    containerUnThreadedMQTopicConsume = NULL;
    this->listRoutingKeyToConsume=paramListRoutingKeyToConsume;
    this->enumSoftware=enum_SoftwareName;
    //monitorProbeAck.set_enum_probesender_software(enum_SoftwareName);

    if(!parseMQParamFile(mqParamFileName))
        exit(1);

    if(!mqIP.startsWith("amqp://"))
    {
        qDebug()<<"AMQP address should start with 'amqp://'. Check if it is set corectly in param_mq.txt. Its current value is: "<<mqIP;
        exit(2);
    }

    if(exchangeNameOut.isEmpty())
        exchangeNameOut=exchangeNameIn;

    startWork();
}

IOMessages::~IOMessages()
{
    // Optional:  Delete all global objects allocated by libprotobuf.
    google::protobuf::ShutdownProtobufLibrary();
}

bool IOMessages::parseMQParamFile(QString mqParamFIleName)
{
    QFile file(mqParamFIleName);
    if(!file.exists())
    {
        qDebug()<<"ERROR: File "<<mqParamFIleName<<"  not exist. Nothing will be done!";
        return false;
    }

    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug()<<"Begin to parse param_mq.txt.........";
        while (!file.atEnd())
        {
            QByteArray line = file.readLine();
            QList <QByteArray> listBa=line.split(',');
            if(line.simplified().isEmpty()||line.simplified().startsWith("/*")||line.simplified().startsWith("//"))
            {
                continue;
            }
            else if(listBa.size()>=2)
            {
                QByteArray paramName=listBa.at(0);
                QByteArray paramValue=listBa.at(1);
                bool ok=true;
                if(paramName.simplified()=="MQIP")
                {
                    mqIP=paramValue;
                    qDebug()<<"MQ IP is: "<<mqIP;
                }
                else if(paramName.simplified()=="ExchangeName")
                {
                    exchangeNameIn=paramValue;
                    qDebug()<<"Exchange name in is:"<<exchangeNameIn;
                }
                else if(paramName.simplified()=="ExchangeNameOut")
                {
                    exchangeNameOut=paramValue;
                    qDebug()<<"Exchange name out is:"<<exchangeNameOut;
                }

                else if(paramName.simplified()=="MQQueueName")
                {
                    mqQueueName=paramValue;
                    qDebug()<<"Queue name is:"<<mqQueueName;
                }

                else if(paramName.simplified()=="ConsumerTag")
                {
                    consumerTag=paramValue;
                    qDebug()<<"consumerTag is:"<<consumerTag;
                }
                else
                    qDebug()<<"Fail to parse field:"+paramName;

                if(!ok)
                    qDebug()<<"Fail to parse value:"+paramValue;
            }
        }
        return true;
    }
    else
    {
        qDebug()<<"ERROR: Fail to open "<<mqParamFIleName<<". Nothing will be done!";
        return false;
    }
}
void IOMessages::startWork()
{
    containerUnThreadedMQTopicPublish = new ContainerOfUnThreadedMQTopicPublish(mqIP,exchangeNameOut,this,300000,60);
    containerUnThreadedMQTopicConsume = new ContainerOfUnThreadedMQTopicConsume(mqIP,exchangeNameIn,listRoutingKeyToConsume,consumerTag,
                                                        this,mqQueueName,300000,600000,"SimuHeartBeat",true,60);
    connect(containerUnThreadedMQTopicConsume,SIGNAL(sigMsgRcvd(QByteArray,QString,QString,quint64,bool)),
            this,SLOT(slotMsgRcvdFromMQ(QByteArray,QString,QString,quint64,bool)));
}


void IOMessages::slotMsgRcvdFromMQ(QByteArray baData, QString exchangeName,
                      QString routingKey, quint64 deliveryTag, bool redelivered)
{
    if(routingKey.startsWith("Monitor."))
    {
       // qDebug()<<"Monitor";

        PBMonitor monitor;
        if(!monitor.ParseFromArray(baData,baData.size()))
        {
            qDebug()<<"Fail to parse pb from string. routingKey is:"<<routingKey<<
                      ".  message's debugstring is:"<<monitor.DebugString().data()
                   <<".  InitializationErrorString is:"<< monitor.InitializationErrorString().data();
        }
        emit sigPBMonitor(monitor);
    }
}

void IOMessages::slotPublishToMQ(QList <StructDataAndKey> listDataAndKey)
{
    containerUnThreadedMQTopicPublish->slotPublishToMQ(listDataAndKey);
}
