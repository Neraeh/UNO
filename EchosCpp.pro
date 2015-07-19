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
    echos.cpp \
    card.cpp \
    cards.cpp \
    deck.cpp \
    players.cpp \
    player.cpp

HEADERS += \
    echos.h \
    card.h \
    cards.h \
    deck.h \
    players.h \
    player.h

include(D:\Dev\src\libcommuni\src\src.pri)
