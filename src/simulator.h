#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <QObject>
#include <QMutex>
#include <QThread>
#include "IOMessages.h"
#include "World.h"
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
    World *world;
    QMutex *mutex;

    QThread *threadOfWorld;
};

#endif // SIMULATOR_H
