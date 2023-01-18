#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QSettings>
#include <QTime>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "server.h"
#include "showtraffic.h"

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
    bool simIsFromPrefs = false;

    long maxDelay = 10 * 1000;

private:
    Ui::MainWindow *ui;
    QSystemTrayIcon *trayIcon;

    QMenu *trayIconMenu;
    QAction *status1Action;
    QAction *status2Action;
    QAction *showTrafficAction;
    QAction *settingsAction;
    QAction *quitAction;

    QNetworkAccessManager networkAccessManager;

    Server server;
    ServerData *serverData = nullptr;
    QDateTime lastReception;
    int timerId;

    QSettings *settings;

    ShowTraffic *showTrafficWindow = nullptr;

    void paintTrayIcon();
    void timerEvent(QTimerEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    void showWindow();
    void showTraffic();

    void iconClicked(QSystemTrayIcon::ActivationReason reason);
    void iconMessageClicked();
    void dismissClicked();

    void secretChanged(QString secret);
    void listItemSelected(QString serial);

    void dataReceived(ServerData *data);
    void parseReply(QNetworkReply *pReply);

    void serverError(QString message);

    void on_radioButton_clicked();

    void on_lineEditURL_editingFinished();
    void on_pushButtonVisit_clicked();

    void on_radioButton_2_clicked();

    void on_spinBoxPort_valueChanged(int arg1);

    void on_pushButtonSecret_clicked();

    void on_lineEditSimId_editingFinished();
    void on_pushButtonList_clicked();

    void on_spinBox_hide_valueChanged(int arg1);

    void on_pushButtonClose_clicked();
};

#endif // MAINWINDOW_H
