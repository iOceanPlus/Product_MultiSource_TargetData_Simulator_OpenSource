#include "LukeEventHandler.h"
#include "ev.h"
#include "amqpcpp.h"
#include "amqpcpp/libev.h"
#include <QDebug>
#include <QDateTime>

LukeEventHandler::LukeEventHandler(struct ev_loop  *loop, bool *isChannelReady, QObject *parent) :
    QObject(parent),AMQP::LibEvHandler(loop)
{
    this->isChannelReady=isChannelReady;
}

void LukeEventHandler::onConnected(AMQP::TcpConnection *connection)
{
    qDebug()<<QDateTime::currentDateTime()<<"MQ is connected. Maxframe:"<<connection->maxFrame();
}

void LukeEventHandler::onError(AMQP::TcpConnection *connection, const char *message)
{
    *isChannelReady=false;
    qDebug()<<QDateTime::currentDateTime()<<":Connection Error occured to MQ:"<<message<<" MaxFrame configured:"<<connection->maxFrame()<<
              " Channels:"<<connection->channels();
    emit sigConnectionError();
}

void LukeEventHandler::onClosed(AMQP::TcpConnection *connection)
{
    *isChannelReady=false;
    qDebug()<<QDateTime::currentDateTime()<<"Connection to MQ closed!"<<" MaxFrame configured:"<<connection->maxFrame()<<
              " Channels:"<<connection->channels();
}

LukeEventHandler::~LukeEventHandler()
{

}

