#include "mainwindow.h"
#include <QApplication>
#include "Logger.h"
#include <QDir>



int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QString currentPath = QCoreApplication::applicationDirPath();
    QDir::setCurrent(currentPath);


    Logger::sharedInstance()->cfg(LoggerLevelInfo, "log/qfacer.log", 20, 30, LoggerModeOnlyBothFileAndQDebug);

    LOG_INFO("set current dir :%s", QStr2CStr(currentPath));

    MainWindow w;
    w.show();

    return a.exec();
}
