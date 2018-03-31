#include "LukeEventHandler.h"
#include "amqpcpp/deferredqueue.h"
#include "ev.h"
#include "amqpcpp.h"
#include "amqpcpp/libev.h"
#include "amqpcpp/channel.h"
#include "amqpcpp/tcpchannel.h"
#include "MQTopicConsumeCore.h"

#include <QDebug>
#include <QObject>
#include <QTimer>
#include <QCoreApplication>
#include <QDateTime>
#include <QMutexLocker>
#include <QThread>

MQTopicConsumeCore::MQTopicConsumeCore(QString mqAddr, QString paramExchangeName, QStringList paramListRoutingKey, QString consumerTag, bool evRunNoWait,
           QObject *parent, QString paramQueueName, int paramMessageInQueueTTL_MiliSeconds, int paramQueueTTL_MiliSeconds,
                                       QString paramRoutingKeyForSimuHeart, bool paramIsQueueDurable) : QObject(parent)
{
    mutex=new QMutex();
    timerRunEv=new QTimer(this);

    this->mqAddr=mqAddr;
    this->exchangeName=paramExchangeName;
    this->listRoutingKey=paramListRoutingKey;
    this->queueName=paramQueueName;
    this->consumerTag=consumerTag;
    this->messageInQueueTTL_MiliSeconds=paramMessageInQueueTTL_MiliSeconds;
    this->queueTTL_Miliseconds=paramQueueTTL_MiliSeconds;
    this->routingKeyForSimuHeart=paramRoutingKeyForSimuHeart;
    this->isQueueDurable=paramIsQueueDurable;
    this->evRunNoWait=evRunNoWait;

    connection=NULL;
    channel=NULL;
    isChannelReady=false;
}

void MQTopicConsumeCore::slotInit()
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
        timerRunEv->start(0);
    }
    else
    {
         auto *loop=  ev_loop_new();
         // handler for libev (so we don't have to implement AMQP::TcpHandler!)
         eventHandler=new LukeEventHandler(loop,&isChannelReady, this);
         connectMQAndDeclare();
         ev_run(loop, 0);
         qDebug()<<"Event loop is exited in MQTopicConsume";
         loop=NULL;

    }
}

void MQTopicConsumeCore::connectMQAndDeclare()
{
    connect(eventHandler,SIGNAL(sigConnectionError()),this,SLOT(slotDestroyAndReInitAfterAWhile()));

    // make a connection
    connection = new AMQP::TcpConnection(eventHandler, AMQP::Address(mqAddr.toStdString()));

    // we need a channel too
    channel = new AMQP::TcpChannel(connection);

    // install a generic channel-error handler that will be called for every
    // error that occurs on the channel
    channel->onError([this](const char *message) {
        QString errorStr="channel error in class MQTopicConsume: " +QString(message);
        emit sigErrorInfo(errorStr);
        qDebug() << errorStr<<endl<<"Begion to reinit....";
        isChannelReady=false;
        this->slotDestroyAndReInitAfterAWhile();
    });

    channel->onReady([this]
    {
        this->isChannelReady=true;
        QString infoStr="Channel is ready in class MQTopicConsume.";
        emit sigInfo(infoStr);
       qDebug()<<infoStr;
    });

    channel->declareExchange(exchangeName.toStdString(),AMQP::topic,AMQP::durable)
        .onSuccess([this] ()
        {
            QString infoStr="Exchange  is declared successfully in MQTopiConsume";
            emit sigInfo(infoStr);
           qDebug()<<infoStr;
        })
        .onError([this] (const char* message)
        {
            QString errorStr="Error when declare exchange  in MQTopiConsume: " +QString(message);
            emit sigErrorInfo(errorStr);
            qDebug()<<errorStr<<endl;
        });

    // callback operation when a queue was declared successfully
    // signature: void myCallback(const std::string &name, uint32_t messageCount, uint32_t consumerCount);
    AMQP::QueueCallback callbackQueueDeclared = [=](const std::string &name, uint32_t messageCount, uint32_t consumerCount)
    {
        QString infoStr="Message queue "+QString(name.data())+" has been declared in class MQTopicConsume.\n"
                "Messagecount is: "+QString::number(messageCount)+". Consumercount is: "+QString::number(consumerCount);
        emit sigInfo(infoStr);
       qDebug()<<infoStr;

        QString routingKey;
        /***Can a nonDurable queue bind with a durable exchange?**/
        foreach (routingKey,listRoutingKey)
            channel->bindQueue(exchangeName.toStdString(),name,routingKey.toStdString());

        // callback function that is called when the consume operation starts
        AMQP::ConsumeCallback callBackConsumeStarted = [this](const std::string &consumertag)
        {
            QString infoStr= "consume operation started. Consumertag is: " +QString(consumertag.data());
            emit sigInfo(infoStr);
            qDebug()<<infoStr;
        };

        // callback function that is called when the consume operation failed
        auto callBackConsumeError = [this](const char *message)
        {
            QString errorStr="Consume operation failed: " +QString(message);
            emit sigErrorInfo(errorStr);
            qDebug() << errorStr;
        } ;

        // callback operation when a message was received
        /***about delibery tag
         * 	The server-assigned and channel-specific delivery tag.
    The delivery tag is valid only within the channel from which the message was received. I.e. a client MUST NOT receive a message on one channel and then acknowledge it on another.
    The server MUST NOT use a zero value for delivery tags. Zero is reserved for client use, meaning "all messages so far received".
********************/
        auto callBackConsumeRecvd = [this](const AMQP::Message &message, uint64_t deliveryTag, bool redelivered)
        {
            QString exchangeName=message.exchange().data();
            QString routingKey=message.routingKey().data();
            //qDebug()<<"msg recvd:"<<routingKey;
            if(!routingKey.startsWith(routingKeyForSimuHeart))
               emit sigMsgRcvd(QByteArray(message.message().data(),message.message().size()),exchangeName,routingKey,deliveryTag,redelivered);
            // acknowledge the message
            //channel->ack(deliveryTag); //noack flag is set
        } ;

        /* start consuming from the queue, and install the callbacks
        *  The following flags are supported:
        *
        *      -   nolocal             if set, messages published on this channel are
        *                              not also consumed
        *
        *      -   noack               if set, consumed messages do not have to be acked,
        *                              this happens automatically
        *
        *      -   exclusive           request exclusive access, only this consumer can
        *                              access the queue
        * ************/
        channel->consume(name, consumerTag.toStdString(), AMQP::exclusive+AMQP::noack+AMQP::nolocal)
         .onReceived(callBackConsumeRecvd)
         .onSuccess(callBackConsumeStarted)
         .onError(callBackConsumeError);
    };

    AMQP::Table arguments;
    /**********************settings of message TTL****************
     * reference: http://www.rabbitmq.com/ttl.html
    arguments["x-dead-letter-exchange"] = "some-exchange";
    参数 x-message-ttl 的值 必须是非负 32 位整数 (0 <= n <= 2^32-1) ，以毫秒为单位表示 TTL 的值。
    这样，值 1000 表示存在于 queue 中的当前 message 将最多只存活 1 秒钟，除非其被投递到 consumer 上
    The argument can be of AMQP type short-short-int, short-int, long-int, or long-long-int.
***************************************************************/

    arguments["x-message-ttl"] = messageInQueueTTL_MiliSeconds;
    std::cout<<std::endl<<"x-message-ttl in the arguments of  queue "<<queueName.toStdString()<<" is:"<<arguments.get("x-message-ttl")<<std::endl;

    //Note1: basic.get(xxx)  provides a direct access to the messages in a queue using a synchronous dialogue that is designed for specific types of application
    //where synchronous functionality is more important than performance.

    /*****settings of queue TTL******
    queue.declare 命令中的 x-expires 参数控制 queue 被自动删除前可以处于未使用状态的时间。
    未使用的意思是 queue 上没有任何 consumer ，queue 没有被重新声明，并且在过期时间段内未调用过 basic.get 命令(see  Note1)。
    该方式可用于，例如，RPC-style 的回复 queue ，其中许多 queue 会被创建出来，但是却从未被使用。
    用于表示超期时间的 x-expires 参数值以milli seconds为单位，并且服从和 x-message-ttl 一样的约束条件，且不能设置为 0 。
    所以，如果该参数设置为 1000 ，则表示该 queue 如果在 1 秒钟之内未被使用则会被删除。
    The server guarantees that the queue will be deleted, if unused for at least the expiration period.
    No guarantee is given as to how promptly the queue will be removed after the expiration period has elapsed.
    Leases of durable queues restart when the server restarts.
    The value of the x-expires argument or expires policy describes the expiration period in milliseconds.
    It must be a positive integer (unlike message TTL it cannot be 0). Thus a value of 1000 means a queue which is unused for 1 second will be deleted.
    arguments["x-expires"] = queueTTL;
    ***********************************************************/
    arguments["x-expires"] = queueTTL_Miliseconds;
    std::cout<<"x-expires in the arguments of queue  "<<queueName.toStdString()<<" is:"<<arguments.get("x-expires")<<std::endl;

    /*******
     * Total body size for ready messages a queue can contain before it starts to drop them from its head.
     * NOT Recommended to set this argument: performance of rabbit is influncenced!
     * ************/
    //arguments["x-max-length-bytes"]=queueMaxLengthBytes;
    //std::cout<<"x-max-length-bytes in the arguments of queue "<<queueName.data()<<" is:"<<arguments.get("x-max-length-bytes")<<std::endl;

    /********************flages in declareQueue*************************
     *  The flags in declareQueue can be a combination of the following values:
    *      -   durable     queue survives a broker restart
    *      -   autodelete  queue is automatically removed when all connected consumers are removed
    *      -   passive     only check if the queue exist,will not create excange
    *      -   exclusive   the queue only exists for this connection, and is automatically removed when connection is gone
    * Just like many others methods in the Channel class, the declareQueue() method accepts an integer parameter named 'flags'.
    * This is a variable in which you can set method-specific options, by summing up all the options that are described in the documentation
    * above the method. If you for example want to create a durable, auto-deleted queue, you can pass in the value AMQP::durable + AMQP::autodelete.
******************************************************************************/
    if(queueName.simplified().isEmpty())
       channel->declareQueue(isQueueDurable?AMQP::durable:AMQP::exclusive,arguments)
               .onSuccess(callbackQueueDeclared)
               .onError([this](const char *message) {
           QString errorStr="Queue declare error in MQTopicConsume:" +QString(message);
           emit sigErrorInfo(errorStr);
           qDebug() <<errorStr;
       });
    else
        channel->declareQueue(queueName.toUtf8().data(), isQueueDurable?AMQP::durable:AMQP::exclusive,arguments)
                .onSuccess(callbackQueueDeclared)
                .onError([this](const char *message) {
            QString errorStr="Queue declare error in MQTopicConsume:" +QString(message);
            emit sigErrorInfo(errorStr);
            qDebug() << errorStr;
        });

}

void MQTopicConsumeCore::slotDestroyAndReInitAfterAWhile()
{
    isChannelReady=false;
    QString infoStr= QDateTime::currentDateTime().toString()+" Destroy and ReInit members in MQTopicConsume after "+
            QString::number(RECONNECT_INTERVAL_SECONDS)+" seconds";
    emit sigInfo(infoStr);
    qDebug()<<infoStr;
    QTimer::singleShot(RECONNECT_INTERVAL_SECONDS*1000,this,SLOT(slotDestroyAndReInitImmediately()));
}

void MQTopicConsumeCore::slotDestroyAndReInitImmediately()
{
    QString infoStr= QDateTime::currentDateTime().toString()+" Trying to ReInit MQTopicConsume ";
    emit sigInfo(infoStr);
    qDebug()<<infoStr;

    if(channel)
    {
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


void MQTopicConsumeCore::slotTimerEventRunEv()
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

//avoid reset caused by missing heartbeat
void MQTopicConsumeCore::slotPublishSimuHeartBeat()
{
    QMutexLocker locker(mutex); //ensure that, this method is not called by two threads simultaneously
    if(channel&&channel->connected()&&isChannelReady)
    {
        if(!channel->publish(exchangeName.toUtf8().data(),routingKeyForSimuHeart.toStdString(),"Hello to Broker"))
        {
            QString errorStr=QDateTime::currentDateTime().toString()+"MQTopicConsumeCore Fail to publish heartbeat to broker!"; //avoid reset caused by missing heartbeat
            emit sigErrorInfo(errorStr);
            qDebug() <<errorStr;
        }
    }
}

MQTopicConsumeCore::~MQTopicConsumeCore()
{
    connection->close();
    channel->close();
    delete connection;
    delete channel;
    delete mutex;
    qDebug()<<"MQTopicConsumeCore destroyed";
}

