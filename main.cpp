/* Copyright (C) 2020 J.Luis <root@heavydeck.net>
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 */
#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QDir>
#include <QSettings>
#include <QDebug>

#include "app-info.h"

int main(int argc, char *argv[])
{
#ifdef QT_DEBUG
    //Debug options, reduce the ammount of magic, make debugging faster.
    QApplication::setAttribute(Qt::AA_UseSoftwareOpenGL, true);
    QApplication::setAttribute(Qt::AA_DontUseNativeDialogs, true);
    QApplication::setAttribute(Qt::AA_DontUseNativeMenuBar, true);
#else
    //Release options
    //(nothing)
#endif

    QApplication a(argc, argv);
    a.setApplicationName(APPLICATION_NAME);
    a.setOrganizationName(ORGANIZATION_NAME);
    a.setOrganizationDomain(ORGANIZATION_DOMAIN);
    a.setApplicationVersion(
                QString::number(VERSION_MAJOR) + "." +
                QString::number(VERSION_MINOR) + "." +
                QString::number(VERSION_BUILD)
                );

    //Translation
    QTranslator myappTranslator;
    {
        QSettings settings;
        QString locale = settings.value(KEY_LANGUAGE, QLocale::system().name()).toString();
        if(locale != ""){
            bool rv = myappTranslator.load("xmodem-send_" + locale, QDir::currentPath() + "/translations");
            a.installTranslator(&myappTranslator);
            if (rv){
                qDebug() << "Added locale for:" << locale;
            }
            else{
                qDebug() << "Failed to add locale for:" << locale << "Current path is:" << QDir::currentPath();
            }
        }
    }

    MainWindow w;
    w.show();

    return a.exec();
}
