#-------------------------------------------------
#
# Project created by QtCreator 2011-12-10T13:05:42
#
#-------------------------------------------------

QT       += core sql

QT       -= gui

TARGET = cute-credit
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    artematimer.cpp \
    reader.cpp \
    fifocontroller.cpp \
    mock.cpp \
    database.cpp \
    artemahybrid.cpp \
    receipttemplate.cpp

HEADERS += \
    helpers.h \
    artematimer.h \
    reader.h \
    cute_credit.h \
    fifocontroller.h \
    mock.h \
    database.h \
    artemahybrid.h \
    paylife_structs.h \
    receipttemplate.h

RESOURCES += \
    cute-credit-resources.qrc
