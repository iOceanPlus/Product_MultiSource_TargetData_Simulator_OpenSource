#ifndef LUKEEVENTHANDLER_H
#define LUKEEVENTHANDLER_H

#include "ev.h"
#include "amqpcpp.h"
#include "amqpcpp/libev.h"
#include <QObject>

class LukeEventHandler : public QObject, public AMQP::LibEvHandler
{
    Q_OBJECT
public:
    explicit LukeEventHandler(struct ev_loop *loop,bool *isChannelReady, QObject *parent = 0);
    ~LukeEventHandler();

public slots:
    virtual void onConnected(AMQP::TcpConnection *connection);
    virtual void onError(AMQP::TcpConnection *connection, const char *message);
    virtual void onClosed(AMQP::TcpConnection *connection);

signals:
    void sigConnectionError();

public slots:

private:
    bool *isChannelReady;
};

#endif // LUKEEVENTHANDLER_H
