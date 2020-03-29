#-------------------------------------------------
#
# Project created by QtCreator 2018-07-21T01:44:09
#
#-------------------------------------------------

QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = xmodem-send
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        crc16-xmodem.c \
        main.cpp \
        mainwindow.cpp \
        transfer.cpp

HEADERS += \
        app-info.h \
        crc16-xmodem.h \
        mainwindow.h \
        transfer.h

RESOURCES +=

TRANSLATIONS = translations/xmodem-send_es.ts
RC_ICONS = icon/vga.ico
