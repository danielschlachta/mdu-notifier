#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QSettings>
#include <QTime>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "server.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    void init(QString simserial);

protected:
    QString sim;

    long maxTransmitAge = 10;
    long maxDelay = 80;
    long transmitInterval = 5;

private:
    Ui::MainWindow *ui;
    QSystemTrayIcon *trayIcon;

    QMenu *trayIconMenu;
    QAction *status1Action;
    QAction *status2Action;
    QAction *settingsAction;
    QAction *quitAction;

    QNetworkAccessManager networkAccessManager;

    Server server;

    int timerId;
    ServerData *serverData = nullptr;

    QTime lastReception;
    QTime *messageShown;
    bool hideBalloon = false;

    QSettings *settings;

    long long inMegabytes(long long mb);
    void setActive();

    void paintTrayIcon();
    void timerEvent(QTimerEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    void showWindow();

    void iconClicked(QSystemTrayIcon::ActivationReason reason);

    void iconMessageClicked();
    void dismissClicked();

    void secretChanged(QString secret);

    void parseReply(QNetworkReply* pReply);

    void serverError(QString message);
    void dataReceived(ServerData data);

    void on_lineEditURL_editingFinished();
    void on_spinBox_show_valueChanged(int arg1);
    void on_spinBox_hide_valueChanged(int arg1);
    void on_pushButtonClose_clicked();
    void on_checkBoxSuppress_clicked();
    void on_pushButtonVisit_clicked();

    void on_checkBoxBuiltin_clicked();
    void on_pushButtonSecret_clicked();
};

#endif // MAINWINDOW_H
