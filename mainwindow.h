#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QTime>

#include "filedownloader.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    long ageThreshold = 0;

private:
    Ui::MainWindow *ui;
    QSystemTrayIcon *trayIcon;

    QMenu *trayIconMenu;
    QAction *settingsAction;
    QAction *quitAction;

    int timerId;
    FileDownloader *pFileDownloader;

    long age = LONG_MAX;
    int percent = 0;
    long long usedBytes = 0;
    long long capBytes = 0;

    QTime lastReception;

    void paintTrayIcon();
    void timerEvent(QTimerEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    void iconMessageClicked();
    void parseReply();
};

#endif // MAINWINDOW_H
