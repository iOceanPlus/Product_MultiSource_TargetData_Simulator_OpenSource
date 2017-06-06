#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <QObject>
#include "IOMessages.h"
#include "ParallelWorld.h"
class Simulator : public QObject
{
    Q_OBJECT
public:
    explicit Simulator(QObject *parent = 0);

signals:

public slots:

private:
    IOMessages *ioMessage;
    ParallelWorld *world;
};

#endif // SIMULATOR_H
