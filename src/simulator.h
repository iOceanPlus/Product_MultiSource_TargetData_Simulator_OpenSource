#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <QObject>
#include <QMutex>
#include <QThread>
#include "IOMessages.h"
#include "ThreadedWorld.h"
class Simulator : public QObject
{
    Q_OBJECT
public:
    explicit Simulator(QObject *parent = 0);
    ~Simulator();
signals:

public slots:

private:
    void connectIOMessageAndWorld();

    IOMessages *ioMessages;
    ThreadedWorld *world;
    QMutex *mutex;

    QThread *threadOfWorld;
};

#endif // SIMULATOR_H
