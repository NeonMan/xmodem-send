#include "mainwindow.h"
#include <QApplication>
#include <QSettings>
#include "app-info.h"

int main(int argc, char *argv[])
{
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
