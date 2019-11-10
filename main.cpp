#include <QWindow>
#include <QApplication>
#include <QSharedMemory>

#include "common.h"

#include "runguard.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    const char *title = APPTITLE;
    const char *version = APPVERSION;

    if (argc == 2 && !strcmp(argv[1], "--version"))
    {
        printf("%s %s built with Qt version %s\n", title, version, qVersion());
        return EXIT_SUCCESS;
    }

    QApplication a(argc, argv);

    a.setQuitOnLastWindowClosed(false);

    MainWindow w;

    if (argc == 3 && !strcmp(argv[1], "--sim"))
        w.sim = argv[2];

    RunGuard runGuard("mdu-notifier-runguard" + w.sim);

    if (!runGuard.tryToRun())
            return EXIT_FAILURE;


    return a.exec();
}
