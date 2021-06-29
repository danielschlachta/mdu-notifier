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
    settings = new QSettings("mdu-notifier", "mdu-notifier");

    ui->setupUi(this);

    ui->lineEditURL->setText(settings->value("url", tr(DEFAULT_URL)).toString());
    ui->spinBoxPort->setValue(settings->value("port", 2434).toInt());

    if (!simserial.isEmpty()) {
        sim = simserial;
        ui->lineEditSimId->setEnabled(false);
    } else {
        sim = settings->value("simserial", "").toString();
        simIsFromPrefs = true;
    }
    ui->lineEditSimId->setText(sim);

    if (settings->value("builtin", false).toBool()) {
        ui->radioButton->setChecked(false);
        ui->radioButton_2->setChecked(true);
        ui->pushButtonList->setEnabled(false);
    } else {
        ui->radioButton->setChecked(true);
        ui->radioButton_2->setChecked(false);
        ui->pushButtonList->setEnabled(simIsFromPrefs);
    }

    ui->spinBox_show->setValue(settings->value("show", 2).toInt());
    ui->spinBox_hide->setValue(settings->value("hide", 10).toInt());
    ui->checkBoxSuppress->setCheckState(settings->value("captime", 0).toLongLong() > 0 ?
                                            Qt::Checked : Qt::Unchecked);
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
    connect(&server, SIGNAL(dataReceived(ServerData*)), this, SLOT(dataReceived(ServerData*)));

    timerId = startTimer(2000);
    timerEvent(nullptr);
}

MainWindow::~MainWindow()
{
    killTimer(timerId);
    delete ui;
    delete settings;

    if (serverData != nullptr)
        delete serverData;
}

void MainWindow::iconClicked(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger && !settings->value("builtin", false).toBool())
        QDesktopServices::openUrl(QUrl(settings->value("url", tr(DEFAULT_URL)).toString()));
}

void MainWindow::iconMessageClicked()
{
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
        if (serverData->active)
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

    int usedPercent = serverData != nullptr && serverData->limit > 0 ?
                static_cast<int>(serverData->used * 100 / serverData->limit) : 0;

    if (usedPercent > 0)
    {
        if (serverData->active)
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

    painter.setCompositionMode(QPainter::CompositionMode_Clear);
    painter.setBrush(QColor("transparent"));
    painter.drawPie(inRect, 90 * 16, 360 * 16);

    QIcon icon(pixmap);
    trayIcon->setIcon(icon);
}

void MainWindow::dataReceived(ServerData *data)
{
    if (data == nullptr)
        return;

    ServerData *oldData = serverData;
    serverData = data;

    if (oldData != nullptr) {
        serverData->active = data->rxtime - oldData->rxtime < maxDelay * 1000;
        delete oldData;
    } else {
        serverData->active = false;
    }

    paintTrayIcon();
}

void MainWindow::parseReply(QNetworkReply* pReply)
{
    QByteArray reply(pReply->readAll());
    pReply->deleteLater();

    QJsonDocument doc = QJsonDocument::fromJson(reply);
    QJsonArray arr = doc.array();
    QJsonObject obj;

    ServerData *newData = nullptr;

    for (int i = 0; i < arr.size(); i++) {
         obj = arr[i].toObject();

         if (obj.value("simserial").toString() == sim) {
             newData = new ServerData;

             long long current = obj.value("current").toString().toLongLong();
             long long floor = obj.value("floor").toString().toLongLong();

             newData->active = false;
             newData->rxtime = obj.value("lastused").toString().toLong();
             newData->used = current - floor;
             newData->limit = obj.value("haslimit").toString() == "1" ? obj.value("limit").toString().toLongLong() : 0;
         }
    }

    dataReceived(newData);
}

void MainWindow::serverError(QString message)
{
    QMessageBox::warning(this, tr(APPTITLE), tr("The built-in server encountered a problem:\n")
                         + message);
    server.hasError = false;
}

void MainWindow::timerEvent(QTimerEvent *event)
{
    if (event != nullptr && event->timerId() != timerId)
        return;

    if (settings->value("builtin", false).toBool())
    {
        if (!server.isOpen && !server.hasError)
            server.open(ui->spinBoxPort->value(), settings->value("secret", "").toString(), sim);
    }
    else
    {
        if (server.isOpen)
            server.close();

        QString urlStr(settings->value("url", DEFAULT_URL).toString());
        if (!urlStr.endsWith("/"))
            urlStr += "/";
        QUrl url = urlStr + "?update=1";

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

    if (serverData != nullptr)
    {
        QString status1;
        QTextStream stream1(&status1);

        stream1 << serverData->used / 1024 / 1024 << " MB";

        long long cap = serverData == nullptr ? 0 : serverData->limit / 1024 / 1024;
        long long remaining = serverData == nullptr ? 0 : cap - serverData->used / 1024 / 1024;

        if (remaining < 0)
            remaining = 0;

        int remainPercent = serverData != nullptr
                && serverData->limit > 0 ? static_cast<int>(remaining * 100 / cap) : 0;

        if (serverData->limit > 0)
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

}

void MainWindow::on_pushButtonClose_clicked()
{
    settings->setValue("pos", pos());
    hide();
}

void MainWindow::on_pushButtonVisit_clicked()
{
    QString urlStr(settings->value("url", DEFAULT_URL).toString());
    if (!urlStr.endsWith("/"))
        urlStr += "/";

    QDesktopServices::openUrl(QUrl(urlStr));
}

void MainWindow::on_pushButtonSecret_clicked()
{
    SecretDialog *secretDialog = new SecretDialog(this, settings->value("secret", "").toString());
    connect(secretDialog, SIGNAL(secretChanged(QString)), this, SLOT(secretChanged(QString)));
    secretDialog->show();
}

void MainWindow::on_radioButton_clicked()
{
    settings->setValue("builtin", false);
    ui->radioButton_2->setChecked(false);
    ui->pushButtonList->setEnabled(true);
    timerEvent(nullptr);
}

void MainWindow::on_radioButton_2_clicked()
{
    server.close();
    settings->setValue("builtin", true);
    ui->radioButton->setChecked(false);
    ui->pushButtonList->setEnabled(false);
    timerEvent(nullptr);
}

void MainWindow::on_spinBoxPort_valueChanged(int arg1)
{
    server.close();
    settings->setValue("port", arg1);
}

void MainWindow::on_lineEditSimId_editingFinished()
{
    settings->setValue("simserial", ui->lineEditSimId->text());

    if (simIsFromPrefs)
        sim = ui->lineEditSimId->text();
}
