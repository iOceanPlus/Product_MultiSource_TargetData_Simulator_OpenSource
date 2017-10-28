QT       += core   positioning
CONFIG += c++11
CONFIG += console
CONFIG -= app_bundle

LIBS +=-L/usr/lib -lprotobuf
LIBS +=-L/usr/local/lib -lev
LIBS +=-L/usr/lib -lamqpcpp

INCLUDEPATH += /usr/local
INCLUDEPATH += $$PWD/TailoredProtoCPP
INCLUDEPATH += $$PWD/Module_Qt_RabbitMQ
INCLUDEPATH += $$PWD/Module_Qt_DBInterface
INCLUDEPATH += $$PWD/TailoredBaseClass_IOMessages
INCLUDEPATH += $$PWD/Module_AISCommon
INCLUDEPATH += $$PWD/Module_MyFixedSizeQueue
INCLUDEPATH += $$PWD/Module_EvidenceTheory
INCLUDEPATH += $$PWD/FuzzyToolbox
INCLUDEPATH += $$PWD/TailoredModule_Qt_PBCoderDecoder
INCLUDEPATH += $$PWD/Module_Qt_Geography

TARGET = TargetDataSimulator

TEMPLATE = app

SOURCES += main.cpp \
    Module_Qt_RabbitMQ/ContainerOfThreadedMQTopicConsume.cpp \
    Module_Qt_RabbitMQ/ContainerOfThreadMQTopicPublish.cpp \
    Module_Qt_RabbitMQ/ContainerOfUnthreadedMQTopicConsume.cpp \
    Module_Qt_RabbitMQ/ContainerOfUnthreadedMQTopicPublish.cpp \
    Module_Qt_RabbitMQ/LukeEventHandler.cpp \
    Module_Qt_RabbitMQ/MQTopicConsumeCore.cpp \
    Module_Qt_RabbitMQ/MQTopicPublishCore.cpp \
    macro.cpp \
    target.cpp \
    simulator.cpp \
    DataChannel.cpp \
    DataSource.cpp \
    PosDevice.cpp \
    World_gridandmq.cpp \
    Module_Qt_Geography/MyGeography.cpp \
    Module_Qt_Geography/MyQtGeoCircle.cpp \
    Module_Qt_Geography/MyQtGeoPolygon.cpp \
    Module_Qt_Geography/MyQtGeoRectangle.cpp \
    Module_Qt_Geography/MyQtGeoShapeBase.cpp \
    ThreadedWorld.cpp \
    TailoredModule_Qt_PBCoderDecoder/PBCoderDecoder.cpp \
    TailoredBaseClass_IOMessages/IOMessages.cpp \
    TailoredProtoCPP/AISData.pb.cc \
    TailoredProtoCPP/CommonEnum.pb.cc \
    TailoredProtoCPP/Monitor.pb.cc \
    TailoredProtoCPP/Target.pb.cc

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

HEADERS += \
    Module_Qt_RabbitMQ/ContainerOfThreadedMQTopicConsume.h \
    Module_Qt_RabbitMQ/ContainerOfThreadMQTopicPublish.h \
    Module_Qt_RabbitMQ/ContainerOfUnThreadedMQTopicpublish.h \
    Module_Qt_RabbitMQ/ContainerOfUntTreadedMQTopicConsume.h \
    Module_Qt_RabbitMQ/LukeEventHandler.h \
    Module_Qt_RabbitMQ/MQTopicConsumeCore.h \
    Module_Qt_RabbitMQ/MQTopicPublishCore.h \
    macro.h \
    target.h \
    simulator.h \
    DataChannel.h \
    DataSource.h \
    PosDevice.h \
    Module_Qt_Geography/MyGeography.h \
    Module_Qt_Geography/MyQtGeoCircle.h \
    Module_Qt_Geography/MyQtGeoPolygon.h \
    Module_Qt_Geography/MyQtGeoRectangle.h \
    Module_Qt_Geography/MyQtGeoShapeBase.h \
    ThreadedWorld.h \
    TailoredModule_Qt_PBCoderDecoder/PBCoderDecoder.h \
    TailoredBaseClass_IOMessages/IOMessages.h \
    TailoredProtoCPP/AISData.pb.h \
    TailoredProtoCPP/CommonEnum.pb.h \
    TailoredProtoCPP/Monitor.pb.h \
    TailoredProtoCPP/Target.pb.h

DISTFILES += \
    TailoredBaseClass_IOMessages/param_mq.txt \
    ../README.md \
    ../LICENSE \
    param.json
