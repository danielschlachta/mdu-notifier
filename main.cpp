#include <QWindow>
#include <QApplication>
#include <QSharedMemory>

#include "runguard.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    RunGuard runGuard("mdu-notifier-runguard");

    if (!runGuard.tryToRun())
            return 0;

    QApplication a(argc, argv);

    a.setQuitOnLastWindowClosed(false);

    MainWindow w;
    return a.exec();
}
