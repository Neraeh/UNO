#-------------------------------------------------
#
# Project created by QtCreator 2015-07-19T15:45:06
#
#-------------------------------------------------

QT       += core network

QT       -= gui

TARGET = EchosCpp
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    chan.cpp \
    irc.cpp \
    user.cpp \
    echos.cpp

HEADERS += \
    chan.h \
    irc.h \
    user.h \
    echos.h
