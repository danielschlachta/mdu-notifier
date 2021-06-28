#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QCloseEvent>
#include <QWindow>
#include <QDesktopServices>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

#include "common.h"
#include "warndialog.h"
#include "secretdialog.h"

#define DEFAULT_URL "http://localhost/mdu"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{}

void MainWindow::init(QString simserial)
{
    sim = simserial;

    QString application = "mdu-notifier";

    if (sim != "")
        application = application + "-" + sim;

    settings = new QSettings("mdu-notifier", application);

    ui->setupUi(this);

    ui->lineEditURL->setText(settings->value("url", tr(DEFAULT_URL)).toString());
    ui->spinBox_show->setValue(settings->value("show", 2).toInt());
    ui->spinBox_hide->setValue(settings->value("hide", 10).toInt());
    ui->checkBoxSuppress->setCheckState(settings->value("captime", 0).toLongLong() > 0 ?
                                            Qt::Checked : Qt::Unchecked);
    ui->checkBoxBuiltin->setCheckState(settings->value("builtin", false).toBool() ?
                                           Qt::Checked : Qt::Unchecked);

    on_checkBoxBuiltin_clicked();

    trayIcon = new QSystemTrayIcon(this);
    paintTrayIcon();

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

#if !defined(Q_OS_WIN)
    trayIconMenu->addAction(status1Action);
    trayIconMenu->addAction(status2Action);
#else
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this,
            SLOT(iconClicked(QSystemTrayIcon::ActivationReason)));
#endif

    trayIconMenu->addAction(settingsAction);
    trayIconMenu->addAction(quitAction);

    connect(trayIcon, &QSystemTrayIcon::messageClicked, this, &MainWindow::iconMessageClicked);
    trayIcon->setContextMenu(trayIconMenu);
    trayIcon->show();

    connect(&networkAccessManager,
            SIGNAL(finished(QNetworkReply*)), this, SLOT(parseReply(QNetworkReply*)));

    connect(&server, SIGNAL(serverError(QString)), this, SLOT(serverError(QString)));
    connect(&server, SIGNAL(dataReceived(ServerData)), this, SLOT(dataReceived(ServerData)));

    lastReception.start();
    timerId = startTimer(1000);

    messageShown = new QTime();
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
    {
        serverData->isActive = serverData->timeElapsed < serverData->transmitInterval + maxDelay
            && lastReception.elapsed() < maxTransmitAge * 1000;

        if (settings->value("captime", 0).toInt() < serverData->capTime
                && ui->checkBoxSuppress->isChecked())
        {
            settings->setValue("captime", static_cast<long long>(serverData->capTime));
            ui->checkBoxSuppress->setCheckState(Qt::Unchecked);

            QTime *oldTime;
            oldTime = messageShown;
            messageShown = new QTime();
            delete oldTime;
        }
    }
}

void MainWindow::iconClicked(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger && !settings->value("builtin", false).toBool())
        QDesktopServices::openUrl(QUrl(settings->value("url", tr(DEFAULT_URL)).toString()));
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

    QColor ltGray(0xd4, 0xcc, 0xc3);
    QColor mdGray(0xb1, 0xa7, 0x9f);
    QColor dkGray(0x80, 0x7c, 0x76);
    QColor ltBlue(0x2c, 0xac, 0xda);
    QColor mdBlue(0x27, 0x7a, 0x97);

    QPainter painter(&pixmap);
    QBrush brush;
    brush.setStyle(Qt::SolidPattern);
    QRectF outRect(20.0, 20.0, 160.0, 160.0);

    if (serverData != nullptr)
    {
        if (serverData->isActive)
        {
            painter.setPen(ltGray);
            brush.setColor(ltGray);
        }
        else
        {
            painter.setPen(mdGray);
            brush.setColor(mdGray);
        }
        painter.setBrush(brush);
    }
    else
    {
        painter.setPen(dkGray);
        brush.setColor(dkGray);
        painter.setBrush(brush);
    }
    painter.drawPie(outRect, 90 * 16, 360 * 16);

    int usedPercent = serverData != nullptr && serverData->capBytes > 0 ?
                static_cast<int>(serverData->usedBytes * 100 / serverData->capBytes) : 0;

    if (usedPercent > 0)
    {
        if (serverData->isActive)
        {
            painter.setPen(ltBlue);
            brush.setColor(ltBlue);
        }
        else
        {
            painter.setPen(mdBlue);
            brush.setColor(mdBlue);
        }
        painter.setBrush(brush);
        painter.drawPie(outRect, 90 * 16, -usedPercent * 360 / 100 * 16);
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

    long long cap = serverData == nullptr ? 0 : inMegabytes(serverData->capBytes);
    long long remaining = serverData == nullptr ? 0 : cap - inMegabytes(serverData->usedBytes);

    if (remaining < 0)
        remaining = 0;

    int remainPercent = serverData != nullptr && serverData->isActive && serverData->capBytes > 0 ?
                static_cast<int>(remaining * 100 / cap) : 0;

    if (serverData != nullptr && serverData->isActive
            && serverData->capBytes > 0 && serverData->warningThreshold > 0
            && remainPercent <= serverData->warningThreshold - 1
            && !ui->checkBoxSuppress->isChecked()
            && show > 0
            && (messageShown->isNull() || messageShown->elapsed() >= show * 1000 * 60))
    {
        QString msg;
        QTextStream stream(&msg);

        stream << remaining << " MB left ("
               << remainPercent << "%)" << endl
               << endl << "This message will appear again in "
               << show << (show > 1 ? " minutes" : " minute");

#if defined(Q_OS_WIN) && (QT_VERSION < 0x050600)
        stream << " unless you click 'Dismiss'.";

        WarnDialog *warnDialog = new WarnDialog(this, msg, settings->value("hide", 0).toInt() * 1000 * 60);
        connect(warnDialog, &WarnDialog::dismissed, this, &MainWindow::dismissClicked);

        warnDialog->show();
        warnDialog->setModal(false);
#else
        stream << " unless you click on it.";

        trayIcon->showMessage(tr("Mobile data is running low"), msg, icon,
                              settings->value("hide", 0).toInt() * 1000);
        hideBalloon = true;
#endif
        messageShown->start();
    }
}

void MainWindow::parseReply(QNetworkReply* pReply)
{
    QByteArray reply(pReply->readAll());
    pReply->deleteLater();

    QJsonDocument doc = QJsonDocument::fromJson(reply);
    QJsonArray arr = doc.array();
    QJsonObject obj = arr[0].toObject();

    ServerData *newData = new ServerData;

    long long current = obj.value("current").toString().toLongLong();
    long long floor = obj.value("floor").toString().toLongLong();
    long long limit = obj.value("haslimit").toString() == "1" ? obj.value("limit").toString().toLongLong() : 0;

    newData->isActive = false;
    newData->timeElapsed = 0;
    newData->capTime = 0;
    newData->transmitInterval = 0;
    newData->warningThreshold = 0;
    newData->displayIEC = 1;
    newData->usedBytes = current - floor;
    newData->capBytes = limit;

    ServerData *oldData = serverData;
    serverData = newData;

    if (oldData != nullptr)
        delete oldData;

    lastReception.restart();
    setActive();

#if defined(Q_OS_WIN) && (QT_VERSION >= 0x050600)
    if (messageShown->elapsed() > settings->value("hide", 0).toInt() * 1000 && hideBalloon)
    {
        trayIcon->hide();
        trayIcon->show();
        hideBalloon = false;
    }
#endif
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
            QUrl url = settings->value("url", DEFAULT_URL).toString() + "/?update=1";

            QNetworkRequest request(url);
            request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

            QJsonObject obj;
            obj["secret"] = settings->value("secret").toString();

            QJsonObject pull;
            pull["type"] = "pull";
            pull["serial"] = sim;
            pull["lastupd"] = 0;
            pull["lastchg"] = 0;

            obj["0"] = pull;

            QJsonDocument doc(obj);

            QByteArray data = doc.toJson();

            networkAccessManager.post(request, data);
        }
    }

    setActive();

    paintTrayIcon();

    if (serverData != nullptr)
    {
        QString status1;
        QTextStream stream1(&status1);

        stream1 << inMegabytes(serverData->usedBytes) << " MB";

        long long cap = serverData == nullptr ? 0 : inMegabytes(serverData->capBytes);
        long long remaining = serverData == nullptr ? 0 : cap - inMegabytes(serverData->usedBytes);

        if (remaining < 0)
            remaining = 0;

        int remainPercent = serverData != nullptr
                && serverData->capBytes > 0 ? static_cast<int>(remaining * 100 / cap) : 0;

        if (serverData->capBytes > 0)
        {
            stream1 << " of " << cap << " MB";

            QString status2;
            QTextStream stream2(&status2);

            stream2 << remaining << " MB left" << " (" << remainPercent << "%)";

            status2Action->setText(status2);
            status2Action->setVisible(true);
            trayIcon->setToolTip(status1 + "\n" + status2);
        }
        else
        {
            status2Action->setVisible(false);
            trayIcon->setToolTip(status1);
        }

        status1Action->setText(status1);
        status1Action->setVisible(true);
    }
    else
    {
        status1Action->setVisible(false);
        status2Action->setVisible(false);
        trayIcon->setToolTip("");
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
