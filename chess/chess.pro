#-------------------------------------------------
#
# Project created by QtCreator 2016-08-30T10:15:26
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = chess
TEMPLATE = app


SOURCES += main.cpp\
        widget.cpp \
    setendpoint.cpp \
    board.cpp \
    engine.cpp

HEADERS  += widget.h \
    setendpoint.h \
    board.h \
    utils.h \
    engine.h

FORMS    += widget.ui \
    setendpoint.ui

RESOURCES += \
    res.qrc
