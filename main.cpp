/* Copyright (C) 2020 J.Luis <root@heavydeck.net>
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 */
#include "mainwindow.h"
#include <QApplication>
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

    MainWindow w;
    w.show();

    return a.exec();
}
