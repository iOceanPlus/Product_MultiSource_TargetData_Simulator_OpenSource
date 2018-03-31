#include "LukeEventHandler.h"
#include "amqpcpp/deferredqueue.h"
#include "ev.h"
#include "amqpcpp.h"
#include "amqpcpp/libev.h"
#include "MQTopicPublishCore.h"
#include <QDebug>
#include <QObject>
#include <QThread>
#include <QTimer>
#include <QDateTime>
#include <QMutexLocker>

MQTopicPublishCore::MQTopicPublishCore(QString mqAddr, QString exchangeName, bool evRunNoWait, QObject *parent, quint32 maxCachedMessageCount)
        : QObject(parent)
{
    this->exchangeName=exchangeName;
    this->mqAddr=mqAddr;
    this->maxCachedMessageCount=maxCachedMessageCount;
    this->evRunNoWait=evRunNoWait;
    timerRunEv=new QTimer(this);

    connection=NULL;
    channel=NULL;
    isChannelReady=false;
    mutex=new QMutex();
}

void MQTopicPublishCore::slotInit()
{
    // access to the event loop

    if(evRunNoWait)
    {
        QMutexLocker locker(mutex); //ensure that, this method is not called by two threads simultaneously
        auto *loop = EV_DEFAULT;
        // handler for libev (so we don't have to implement AMQP::TcpHandler!)
        eventHandler=new LukeEventHandler(loop,&isChannelReady, this);
        connectMQAndDeclare();
        ev_run(loop,EVRUN_NOWAIT);

        connect(timerRunEv,SIGNAL(timeout()),this,SLOT(slotTimerEventRunEv()));
        timerRunEv->start(50);
    }
    else
    {
         auto *loop=  ev_loop_new();
         // handler for libev (so we don't have to implement AMQP::TcpHandler!)
         eventHandler=new LukeEventHandler(loop,&isChannelReady, this);
         connectMQAndDeclare();
         ev_run(loop, 0);
         qDebug()<<"Event loop is exited in MQTopicPublish";
         loop=NULL;
    }
}

void MQTopicPublishCore::connectMQAndDeclare()
{
    connect(eventHandler,SIGNAL(sigConnectionError()),this,SLOT(slotDestroyAndReInitAfterAWhile()));

    // make a connection
    connection = new AMQP::TcpConnection(eventHandler, AMQP::Address(mqAddr.toStdString()));

    // we need a channel too
    channel = new AMQP::TcpChannel(connection);

    // install a generic channel-error handler that will be called for every
    // error that occurs on the channel
    channel->onError([this](const char *message) {
        QString errorStr=QDateTime::currentDateTime().toString()+"channel error in class MQTopicPublish: " +QString(message);
        emit sigErrorInfo(errorStr);
        qDebug() <<errorStr;
        isChannelReady=false;
        this->slotDestroyAndReInitAfterAWhile();
    });
    channel->onReady([this]
    {
        this->isChannelReady=true;
        QString infoStr="Channel is ready in class MQTopicPublish.";
        emit sigInfo(infoStr);
       qDebug()<<infoStr;
    });

    /*  The following flags can be used for the exchange:
    *
    *      -   durable     exchange survives a broker restart
    *      -   autodelete  exchange is automatically removed when all connected queues are removed
    *      -   passive     only check if the exchange exist,will not create excange
*/
    channel->declareExchange(exchangeName.toStdString(),AMQP::topic,AMQP::durable);
}


void MQTopicPublishCore::slotPublish(const QList<StructDataAndKey> &listDataAndKey)
{
    QMutexLocker locker(mutex); //ensure that, this method is not called by two threads simultaneously
    listDataAndKeyToBePublished.append(listDataAndKey);

    if(connection&&channel&&channel->connected()&&isChannelReady)
    {
        while (!listDataAndKeyToBePublished.isEmpty())
        {
            StructDataAndKey dataAndKey=listDataAndKeyToBePublished.first();
            // If per message TTL is required (not recommended!), use:     bool publish(const std::string &exchange, const std::string &routingKey, const char *message, size_t size)
            // message.setExpiration("xxxx") can be used to set the expiration.
            if(channel->publish(exchangeName.toStdString(),dataAndKey.routingKey.toStdString(),
                                dataAndKey.data.data(),dataAndKey.data.size()))
                listDataAndKeyToBePublished.removeFirst();
            else
                break;
        }
    }

    quint32 messageCount=listDataAndKeyToBePublished.size();
    if(messageCount>maxCachedMessageCount)
    {
        qint32 countToRemove=messageCount-maxCachedMessageCount;
        while(countToRemove>0)
        {
            listDataAndKeyToBePublished.removeFirst();
            countToRemove--;
        }
    }
}

void MQTopicPublishCore::slotTimerEventRunEv()
{
    auto *loop = EV_DEFAULT;
    /**** ev_run flags values
    enum {
      EVRUN_NOWAIT = 1, // do not block/wait
      EVRUN_ONCE   = 2  // block once only
    };
    ***/
    ev_run(loop, EVRUN_NOWAIT); //NOWAIT
}

void MQTopicPublishCore::slotDestroyAndReInitAfterAWhile()
{
    QString infoStr= QDateTime::currentDateTime().toString()+" Destroy and ReInit members in MQTopicPublish after "+
            QString::number(RECONNECT_INTERVAL_SECONDS)+" seconds";
    emit sigInfo(infoStr);
    qDebug()<<infoStr;

    isChannelReady=false;
    QTimer::singleShot(RECONNECT_INTERVAL_SECONDS*1000,this,SLOT(slotDestroyAndReInitImmediately()));
}

void MQTopicPublishCore::slotDestroyAndReInitImmediately()
{
    QString infoStr= QDateTime::currentDateTime().toString()+" Trying to ReInit MQTopicPublish ";
    emit sigInfo(infoStr);
    qDebug()<<infoStr;

    isChannelReady=false;

    if(channel)
    {
        channel->close();
        delete channel;
        channel=NULL;
    }
    if(connection)
    {
        connection->close();
        delete connection;
        connection=NULL;
    }
    if(eventHandler)
    {
        delete eventHandler;
        eventHandler=NULL;
    }

    slotInit();
}

void MQTopicPublishCore::slotTimerEventSimuHeartBeat()
{
    QMutexLocker locker(mutex); //ensure that, this method is not called by two threads simultaneously

    if(channel&&channel->connected()&&isChannelReady)
    {
        if(!channel->publish(exchangeName.toUtf8().data(),"Simu.HeartBeat","Hello to Broker"))
        {
            QString errorStr=QDateTime::currentDateTime().toString()+"MQTopicPublish Fail to publish heartbeat to broker!"; //avoid reset caused by missing heartbeat
            emit sigErrorInfo(errorStr);
            qDebug() <<errorStr;
        }
    }
}

MQTopicPublishCore::~MQTopicPublishCore()
{
    channel->close();
    connection->close();
    delete connection;
    delete channel;
    delete mutex;
    qDebug()<<"MQTopicPublishCore destroyed";

}
