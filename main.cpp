#include <QWindow>
#include <QApplication>
#include <QSharedMemory>

#include "common.h"

#include "runguard.h"
#include "mainwindow.h"

#define SIM "--sim="

int main(int argc, char *argv[])
{
    const char *title = APPTITLE;
    const char *version = APPVERSION;
    QString sim;

    if (argc == 2 && !strcmp(argv[1], "--version"))
    {
        printf("%s %s built with Qt version %s\n", title, version, qVersion());
        return EXIT_SUCCESS;
    }

    if (argc == 2 && !strncmp(argv[1], SIM, 6))
    {
        sim = argv[1];
        sim.remove(SIM);
    }

    QApplication a(argc, argv);

    a.setQuitOnLastWindowClosed(false);

    MainWindow w;
    w.init(sim);

    RunGuard runGuard("mdu-notifier-runguard" + sim);

    if (!runGuard.tryToRun())
            return EXIT_FAILURE;

    return a.exec();
}
