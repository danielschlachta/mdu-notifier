#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QCloseEvent>
#include <QWindow>
#include <QDesktopServices>

#include "common.h"
#include "warndialog.h"
#include "secretdialog.h"

#define DEFAULT_URL "http://localhost/mdu"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    settings = new QSettings("mdu-notifier", "mdu-notifier");

    ui->setupUi(this);

    ui->lineEditURL->setText(settings->value("url", tr(DEFAULT_URL)).toString());
    ui->spinBox_show->setValue(settings->value("show", 2).toInt());
    ui->spinBox_hide->setValue(settings->value("hide", 10).toInt());
    ui->checkBoxSuppress->setCheckState(settings->value("captime", 0).toLongLong() > 0 ?
                                            Qt::Checked : Qt::Unchecked);
    ui->checkBoxBuiltin->setCheckState(settings->value("builtin", false).toBool() ?
                                           Qt::Checked : Qt::Unchecked);

    on_checkBoxBuiltin_clicked();

    status1Action = new QAction(this);
    status1Action->setEnabled(false);
    status1Action->setVisible(false);
    status2Action = new QAction(this);
    status2Action->setEnabled(false);
    status2Action->setVisible(false);

    settingsAction = new QAction(tr("Preferences..."), this);
    connect(settingsAction, &QAction::triggered, this, &MainWindow::showWindow);

    quitAction = new QAction(tr("&Quit " APPTITLE), this);
    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);

    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(status1Action);
    trayIconMenu->addAction(status2Action);
    trayIconMenu->addAction(settingsAction);
    trayIconMenu->addAction(quitAction);

    trayIcon = new QSystemTrayIcon(this);
    paintTrayIcon();

    connect(trayIcon, &QSystemTrayIcon::messageClicked, this, &MainWindow::iconMessageClicked);   
    trayIcon->setContextMenu(trayIconMenu);
    trayIcon->show();

    connect(&networkAccessManager,
            SIGNAL(finished(QNetworkReply*)), this, SLOT(parseReply(QNetworkReply*)));

    connect(&server, SIGNAL(serverError(QString)), this, SLOT(serverError(QString)));
    connect(&server, SIGNAL(dataReceived(ServerData)), this, SLOT(dataReceived(ServerData)));

    lastReception.start();
    timerId = startTimer(1000);
}

MainWindow::~MainWindow()
{
    killTimer(timerId);
    delete ui;
    delete settings;

    if (serverData != nullptr)
        delete serverData;
}

long long MainWindow::inMegabytes(long long mb)
{
    return serverData == nullptr ? 0 : serverData->displayIEC ? mb / 1024 / 1024 : mb / 1000 / 1000;
}

void MainWindow::setActive()
{
    if (serverData != nullptr)
        serverData->isActive = serverData->timeElapsed < serverData->transmitInterval + maxDelay
            && lastReception.elapsed() < maxTransmitAge * 1000;
}

void MainWindow::iconMessageClicked()
{
    settings->setValue("captime", static_cast<long long>(serverData->capTime));
    ui->checkBoxSuppress->setCheckState(Qt::Checked);
}

void MainWindow::dismissClicked()
{
    iconMessageClicked();
}

void MainWindow::secretChanged(QString secret)
{
    settings->setValue("secret", secret);
}

void MainWindow::showWindow()
{
    move(settings->value("pos", QPoint(200, 200)).toPoint());
    showNormal();
    raise();
}

void MainWindow::paintTrayIcon()
{
    QPixmap pixmap(200, 200);
    pixmap.fill(Qt::transparent);

    QColor ltBlue(0x2c, 0xac, 0xda);
    QColor ltGray(0xd4, 0xcc, 0xc3);
    QColor dkGray(0x80, 0x7c, 0x76);

    QPainter painter(&pixmap);
    QBrush brush;
    brush.setStyle(Qt::SolidPattern);
    QRectF outRect(20.0, 20.0, 160.0, 160.0);

    if (serverData != nullptr && serverData->isActive)
    {
        painter.setPen(ltGray);
        brush.setColor(ltGray);
        painter.setBrush(brush);
    }
    else
    {
        painter.setPen(dkGray);
        brush.setColor(dkGray);
        painter.setBrush(brush);
    }
    painter.drawPie(outRect, 90 * 16, 360 * 16);

    int percent = serverData != nullptr && serverData->isActive && serverData->capBytes > 0 ?
                static_cast<int>(serverData->usedBytes * 100 / serverData->capBytes) : 0;

    if (percent > 0)
    {
        painter.setPen(ltBlue);
        brush.setColor(ltBlue);
        painter.setBrush(brush);
        painter.drawPie(outRect, 90 * 16, -percent * 360 / 100 * 16);
    }

    double inRad = 50.0;

    QRectF inRect(outRect.left() + inRad, outRect.top() + inRad,
                  outRect.width() - inRad * 2, outRect.height() - inRad * 2);

    brush.setColor(QColor(70, 70, 70));
    painter.setBrush(brush);
    painter.drawPie(inRect, 90 * 16, 360 * 16);

    QIcon icon(pixmap);
    trayIcon->setIcon(icon);

    int show = settings->value("show", 0).toInt();
    long long remaining = serverData == nullptr ? 0 : serverData->capBytes - serverData->usedBytes;

    if (remaining < 0)
        remaining = 0;

    if (serverData != nullptr && serverData->isActive && serverData->capBytes > 0 && serverData->warningThreshold > 0
            && remaining * 100 / serverData->capBytes <= serverData->warningThreshold - 1
            && settings->value("captime", 0).toInt() < serverData->capTime
            && show > 0
            && (messageShown.elapsed() == 0 || messageShown.elapsed() >= show * 1000 * 60))
    {
        QString msg;
        QTextStream stream(&msg);
        stream << inMegabytes(remaining) << " MB remaining ("
               << remaining * 100 / serverData->capBytes << "%)" << endl
               << endl << "This message will appear again in "
               << show << (show > 1 ? " minutes" : " minute");

#if defined(Q_OS_WIN) && (QT_VERSION <= 0x050500)
        stream << " unless you click 'Dismiss'.";

        WarnDialog *warnDialog = new WarnDialog(this, msg, settings->value("hide", 0).toInt() * 1000 * 60);
        connect(warnDialog, &WarnDialog::dismissed, this, &MainWindow::dismissClicked);

        warnDialog->show();
        warnDialog->setModal(false);
#else
        stream << " unless you click on it.";

        trayIcon->showMessage(tr("Mobile data is running low"), msg, icon,
                              settings->value("hide", 0).toInt() * 1000);
#endif
        messageShown.start();
    }
}

void MainWindow::parseReply(QNetworkReply* pReply)
{
    QByteArray reply(pReply->readAll());
    pReply->deleteLater();

    QStringList data = tr(reply.data()).split("/");

    if (data.count() == 7)
    {
        ServerData *newData = new ServerData;

        newData->isActive = false;
        newData->timeElapsed = data.value(0).toLong();
        newData->capTime = data.value(1).toLong();
        newData->transmitInterval = data.value(2).toInt();
        newData->warningThreshold = data.value(3).toInt();
        newData->displayIEC = data.value(4).toInt() != 0;
        newData->usedBytes = data.value(5).toLongLong();
        newData->capBytes = data.value(6).toLongLong();

        ServerData *oldData = serverData;
        serverData = newData;

        if (oldData != nullptr)
            delete oldData;

        lastReception.restart();
        setActive();
    }
}

void MainWindow::serverError(QString message)
{
    QMessageBox::warning(this, tr(APPTITLE), tr("The built-in server encountered a problem:\n")
                         + message);
    server.hasError = false;
}

void MainWindow::dataReceived(ServerData data)
{
    ServerData *newData = new ServerData;
    *newData = data;

    ServerData *oldData = serverData;
    serverData = newData;

    if (serverData != nullptr)
        delete oldData;

    lastReception.restart();
    setActive();
}

void MainWindow::timerEvent(QTimerEvent *event)
{
    if (event->timerId() != timerId)
        return;

    if (ui->checkBoxBuiltin->isChecked())
    {
        if (!server.isOpen && !server.hasError)
            server.open(ui->spinBoxPort->value(), settings->value("secret", "").toString());
    }
    else
    {
        if (server.isOpen)
            server.close();

        if (lastReception.elapsed() >= transmitInterval)
        {
            QUrl url = settings->value("url", DEFAULT_URL).toString() + "/mdu-notifier.php";
            QNetworkRequest request(url);
            networkAccessManager.get(request);
        }
    }

    setActive();

    paintTrayIcon();

    if (serverData != nullptr && serverData->isActive)
    {
        QString status1;
        QTextStream stream1(&status1);
        stream1 << inMegabytes(serverData->usedBytes) << " MB";

        if (serverData->capBytes > 0)
        {
            stream1 << " of " << inMegabytes(serverData->capBytes) << " MB";

            QString status2;
            QTextStream stream2(&status2);

            stream2 << (serverData->capBytes > serverData->usedBytes ?
                        inMegabytes(serverData->capBytes)  - inMegabytes(serverData->usedBytes) : 0) << " MB left"
                    << " (" << (serverData->capBytes - serverData->usedBytes) * 100 / serverData->capBytes << "%)";

            status2Action->setText(status2);
            status2Action->setVisible(true);
        }
        else
            status2Action->setVisible(false);

        status1Action->setText(status1);
        status1Action->setVisible(true);

    }
    else
    {
        status1Action->setVisible(false);
        status2Action->setVisible(false);
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    settings->setValue("pos", pos());
    hide();
    event->ignore();
}

void MainWindow::on_lineEditURL_editingFinished()
{
    settings->setValue("url", ui->lineEditURL->text());
    lastReception.setHMS(1, 0, 0);
}

void MainWindow::on_spinBox_show_valueChanged(int arg1)
{
    settings->setValue("show", arg1);
}

void MainWindow::on_spinBox_hide_valueChanged(int arg1)
{
    settings->setValue("hide", arg1);
}

void MainWindow::on_checkBoxSuppress_clicked()
{
    if (ui->checkBoxSuppress->isChecked())
        settings->setValue("captime", static_cast<long long>(serverData->capTime));
    else
        settings->setValue("captime", 0);
}

void MainWindow::on_pushButtonClose_clicked()
{
    settings->setValue("pos", pos());
    hide();
}

void MainWindow::on_pushButtonVisit_clicked()
{
    QDesktopServices::openUrl(QUrl(ui->lineEditURL->text()));
}

void MainWindow::on_checkBoxBuiltin_clicked()
{
    if (ui->checkBoxBuiltin->isChecked())
    {
        ui->lineEditURL->setEnabled(false);
        ui->pushButtonVisit->setEnabled(false);
        ui->spinBoxPort->setEnabled(false);
        settings->setValue("builtin", true);
    }
    else
    {
        ui->lineEditURL->setEnabled(true);
        ui->pushButtonVisit->setEnabled(true);
        ui->spinBoxPort->setEnabled(true);
        settings->setValue("builtin", false);
    }
}

void MainWindow::on_pushButtonSecret_clicked()
{
    SecretDialog *secretDialog = new SecretDialog(this, settings->value("secret", "").toString());
    connect(secretDialog, SIGNAL(secretChanged(QString)), this, SLOT(secretChanged(QString)));
    secretDialog->show();
}
