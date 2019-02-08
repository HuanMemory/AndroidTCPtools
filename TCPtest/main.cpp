#include "mainwindow.h"
#include <QApplication>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName("DLUTSmartcar");
    QCoreApplication::setOrganizationDomain("dlut.edu.cn");
    QCoreApplication::setApplicationName("DLUTSmartcarTCPtools");
    MainWindow w;
    w.show();
    return a.exec();
}
