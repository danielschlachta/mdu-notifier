#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QTime>
#include <QNetworkAccessManager>
#include <QNetworkReply>

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
    long maxTransmitAge = 10;
    long maxDelay = 15;
    long transmitInterval = 5;
    QUrl url;

private:
    Ui::MainWindow *ui;
    QSystemTrayIcon *trayIcon;

    QMenu *trayIconMenu;
    QAction *settingsAction;
    QAction *quitAction;

    QNetworkAccessManager networkAccessManager;

    int timerId;

    long age = LONG_MAX;
    int interval= 5;
    int warn = 0;
    long long usedBytes = 0;
    long long capBytes = 0;

    int percent = 0;

    QTime lastReception;

    void paintTrayIcon();
    void timerEvent(QTimerEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    void iconMessageClicked();
    void parseReply(QNetworkReply* pReply);
};

#endif // MAINWINDOW_H
