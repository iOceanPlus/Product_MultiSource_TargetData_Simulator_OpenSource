#include <QMutex>
#include <QDateTime>
#include <QFile>
#include <QDebug>
#include "ContainerOfUnThreadedMQTopicpublish.h"
#include "ContainerOfUntTreadedMQTopicConsume.h"
#include "IOMessages.h"

#include <exception>
using namespace std;

IOMessages::IOMessages(const PB_Enum_Software &enum_SoftwareName , const QStringList &listRoutingKeyToConsume,
                       const QString &mqParamFileName, const bool &threaded, QObject *parent) :
    QObject(parent)
{
    mutex=new QMutex();
    isContainerThreaded=threaded;
    containerUnThreadedMQTopicPublish = NULL;
    containerUnThreadedMQTopicConsume = NULL;
    containerThreadedMQTopicPublish = NULL;
    containerThreadedMQTopicConsume = NULL;

    outPutMsg=false;
    outPutTargetPosInMsg=false;
    debugTimeStamp=false;
    this->enumSoftware=enum_SoftwareName;
    this->structMQParams.listRoutingKeyToConsume=listRoutingKeyToConsume;

    if(mqParamFileName.isEmpty()|| !parseMQParamFile(mqParamFileName))
    {
        qDebug()<<"mqParamFileName not exist or fail to parseMQParamFile";
        exit(1);
    }

    if(!structMQParams.mqAddr.startsWith("amqp://"))
    {
        qDebug()<<"AMQP address should start with 'amqp://'. Check if it is set corectly in param_mq.txt. Its current value is: "<<structMQParams.mqAddr;
        exit(2);
    }

    if(structMQParams.exchangeNameOut.isEmpty())
        structMQParams.exchangeNameOut=structMQParams.exchangeNameIn;

    startWork();
}

IOMessages::IOMessages(const PB_Enum_Software &enum_SoftwareName , const StructMQParams &structMQParamsPassedIn,
                      const bool &threaded,  QObject *parent ) :
    QObject(parent)
{
    mutex=new QMutex();
    isContainerThreaded=threaded;

    containerUnThreadedMQTopicPublish = NULL;
    containerUnThreadedMQTopicConsume = NULL;
    containerThreadedMQTopicPublish = NULL;
    containerThreadedMQTopicConsume = NULL;
    outPutMsg=false;
    outPutTargetPosInMsg=false;
    debugTimeStamp=false;
    this->structMQParams=structMQParamsPassedIn;
    this->enumSoftware=enum_SoftwareName;

    if(!structMQParams.mqAddr.startsWith("amqp://"))
    {
        qDebug()<<"AMQP address should start with 'amqp://'. Check if it is set corectly in param_mq.txt. Its current value is: "<<structMQParams.mqAddr;
        exit(2);
    }

    if(structMQParams.exchangeNameOut.isEmpty())
        structMQParams.exchangeNameOut=structMQParams.exchangeNameIn;

    startWork();
}

IOMessages::~IOMessages()
{
    // Optional:  Delete all global objects allocated by libprotobuf.
    google::protobuf::ShutdownProtobufLibrary();
    delete mutex;
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
                    structMQParams.mqAddr=paramValue;
                    qDebug()<<"MQ IP is: "<<structMQParams.mqAddr;
                }
                else if(paramName.simplified()=="ExchangeName")
                {
                    structMQParams.exchangeNameIn=paramValue;
                    qDebug()<<"Exchange name in is:"<<structMQParams.exchangeNameIn;
                }
                else if(paramName.simplified()=="ExchangeNameOut")
                {
                    structMQParams.exchangeNameOut=paramValue;
                    qDebug()<<"Exchange name out is:"<<structMQParams.exchangeNameOut;
                }

                else if(paramName.simplified()=="MQQueueName")
                {
                    structMQParams.mqQueueName=paramValue;
                    qDebug()<<"Queue name is:"<<structMQParams.mqQueueName;
                }

                else if(paramName.simplified()=="ConsumerTag")
                {
                    structMQParams.consumerTag=paramValue;
                    qDebug()<<"consumerTag is:"<<structMQParams.consumerTag;
                }

                else if(paramName.simplified()=="OutPutMsg")
                {
                    outPutMsg=paramValue.toInt()>0;
                    qDebug()<<"outPutMsg is:"<<outPutMsg;
                }

                else if(paramName.simplified()=="OutPutTargetPosInMsg")
                {
                    outPutTargetPosInMsg=paramValue.toInt(&ok)>0;
                    qDebug()<<"OutPutTargetPosInMsg is:"<<outPutTargetPosInMsg;
                }
                else if(paramName.simplified()=="DebugTimeStamp")
                {
                    debugTimeStamp=paramValue.toInt(&ok)>0;
                    qDebug()<<"DebugTimeStamp is:"<<debugTimeStamp;
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
    if(!isContainerThreaded)
    {
        containerUnThreadedMQTopicPublish = new ContainerOfUnThreadedMQTopicPublish(structMQParams.mqAddr,
                                                                                    structMQParams.exchangeNameOut,this,300000,6);
        containerUnThreadedMQTopicConsume = new ContainerOfUnThreadedMQTopicConsume(structMQParams.mqAddr,
                                            structMQParams.exchangeNameIn,structMQParams.listRoutingKeyToConsume,structMQParams.consumerTag,
                                                            this,structMQParams.mqQueueName,300000,600000,"SimuHeartBeat",true,60);
        connect(containerUnThreadedMQTopicConsume,SIGNAL(sigMsgRcvd(QByteArray,QString,QString,quint64,bool)),
                this,SLOT(slotMsgRcvdFromMQ(QByteArray,QString,QString,quint64,bool)));
        connect(containerUnThreadedMQTopicConsume,SIGNAL(sigErrorInfo(QString)),this,SIGNAL(sigErrorInfo(QString)));
        connect(containerUnThreadedMQTopicConsume,SIGNAL(sigInfo(QString)),this,SIGNAL(sigInfo(QString)));
    }
    else
    {
        containerThreadedMQTopicPublish = new ContainerOfThreadMQTopicPublish(structMQParams.mqAddr,
                                                                                    structMQParams.exchangeNameOut,this,300000,6);
        containerThreadedMQTopicConsume = new ContainerOfThreadedMQTopicConsume(structMQParams.mqAddr,
                                            structMQParams.exchangeNameIn,structMQParams.listRoutingKeyToConsume,structMQParams.consumerTag,
                                                            this,structMQParams.mqQueueName,300000,600000,"SimuHeartBeat",true,60);
        connect(containerThreadedMQTopicConsume,SIGNAL(sigMsgRcvd(QByteArray,QString,QString,quint64,bool)),
                this,SLOT(slotMsgRcvdFromMQ(QByteArray,QString,QString,quint64,bool)));
        connect(containerThreadedMQTopicConsume,SIGNAL(sigErrorInfo(QString)),this,SIGNAL(sigErrorInfo(QString)));
        connect(containerThreadedMQTopicConsume,SIGNAL(sigInfo(QString)),this,SIGNAL(sigInfo(QString)));
        connect(this,SIGNAL(sigPublishToMQ(QList<StructDataAndKey>)),containerThreadedMQTopicPublish,
                SLOT(slotPublishToMQ(QList<StructDataAndKey>)));
    }
}


void IOMessages::slotMsgRcvdFromMQ(const QByteArray &baData, const QString &exchangeName,
                   const   QString &routingKey,const quint64 &deliveryTag, const bool &redelivered)
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

void IOMessages::slotPublishToMQ(const QList<StructDataAndKey> &listDataAndKey) const
{
    if(!isContainerThreaded)
        containerUnThreadedMQTopicPublish->slotPublishToMQ(listDataAndKey);
    else
    {
        emit sigPublishToMQ(listDataAndKey);
    }
}

