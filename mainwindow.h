#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QSettings>
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

private:
    Ui::MainWindow *ui;
    QSystemTrayIcon *trayIcon;

    QMenu *trayIconMenu;
    QAction *status1Action;
    QAction *status2Action;
    QAction *settingsAction;
    QAction *quitAction;

    QNetworkAccessManager networkAccessManager;

    int timerId;

    // shared with QAction
    long recvDelay = LONG_MAX;
    long capTime = 0;
    int interval= 5;
    int warn = 0;
    bool displayMetric = false;
    long long usedBytes = 0;
    long long capBytes = 0;

    bool active = false;

    QTime lastReception;
    QTime messageShown;

    QSettings *settings;

    long long inMegabytes(long long mb);

    void paintTrayIcon();
    void timerEvent(QTimerEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    void showWindow();

    void iconMessageClicked();
    void dismissClicked();

    void parseReply(QNetworkReply* pReply);

    void on_lineEditURL_editingFinished();
    void on_spinBox_show_valueChanged(int arg1);
    void on_spinBox_hide_valueChanged(int arg1);
    void on_pushButtonClose_clicked();
    void on_checkBoxSuppress_clicked();
    void on_pushButtonVisit_clicked();

};

#endif // MAINWINDOW_H
