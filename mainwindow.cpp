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
#include "listdialog.h"
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

    this->setWindowTitle("Preferences - " + this->windowTitle());

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
        ui->lineEditURL->setEnabled(false);
        ui->pushButtonVisit->setEnabled(false);
        ui->lineEditSimId->setEnabled(false);
        ui->pushButtonList->setEnabled(false);
    } else {
        ui->radioButton->setChecked(true);
        ui->radioButton_2->setChecked(false);
        ui->pushButtonList->setEnabled(simIsFromPrefs);
        ui->spinBoxPort->setEnabled(false);
    }

    ui->spinBox_hide->setValue(settings->value("hide", 10).toInt());

    trayIcon = new QSystemTrayIcon(this);
    paintTrayIcon();

    status1Action = new QAction(this);
    status1Action->setEnabled(false);
    status1Action->setVisible(false);
    status2Action = new QAction(this);
    status2Action->setEnabled(false);
    status2Action->setVisible(false);

    showTrafficAction = new QAction(tr("Show Traffic..."), this);
    connect(showTrafficAction, &QAction::triggered, this, &MainWindow::showTraffic);
    showTrafficAction->setEnabled(false);

    settingsAction = new QAction(tr("Preferences..."), this);
    connect(settingsAction, &QAction::triggered, this, &MainWindow::showWindow);

    quitAction = new QAction(tr("&Quit " APPTITLE), this);
    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);

    trayIconMenu = new QMenu(this);

#if !defined(Q_OS_WIN)
    trayIconMenu->addAction(status1Action);
    trayIconMenu->addAction(status2Action);
    trayIconMenu->addAction(showTrafficAction);
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

    showTrafficWindow = new ShowTraffic(this, settings);

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
    if (reason == QSystemTrayIcon::Trigger && showTrafficAction->isEnabled())
        showTraffic();
}

void MainWindow::iconMessageClicked()
{
    //ui->checkBoxSuppress->setCheckState(Qt::Checked);
}

void MainWindow::dismissClicked()
{
    iconMessageClicked();
}

void MainWindow::secretChanged(QString secret)
{
    settings->setValue("secret", secret);
}

void MainWindow::listItemSelected(QString serial)
{
    this->ui->lineEditSimId->setText(serial);
    on_lineEditSimId_editingFinished();
}

void MainWindow::showWindow()
{
    move(settings->value("pos", QPoint(200, 200)).toPoint());
    showNormal();
    raise();
}

void MainWindow::showTraffic()
{
    showTrafficWindow->move(settings->value("trafficpos", QPoint(400, 250)).toPoint());
    showTrafficWindow->resize(settings->value("trafficsize", QSize(400, 650)).toSize());
    showTrafficWindow->show();
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
    if (data != nullptr) {
        long long currTime = QDateTime::currentDateTime().toTime_t() * 1000LL;
        data->active = currTime - data->lastUpdate < maxDelay;

        ServerData *oldData = serverData;
        serverData = data;

        if (oldData != nullptr)
            delete oldData;
    } else
        if (serverData != nullptr)
            serverData->active = false;

    paintTrayIcon();

    showTrafficAction->setEnabled(serverData != nullptr);

    if (serverData == nullptr) {
        status1Action->setVisible(false);
        status2Action->setVisible(false);
        trayIcon->setToolTip("");
        return;
    }

    if (showTrafficWindow != nullptr) {
        showTrafficWindow->updateData(*serverData);
    }

    lastReception = QDateTime::currentDateTime();

    QString status1;
    QTextStream stream1(&status1);

    stream1 << serverData->used / 1024 / 1024 << " MB";

    long long cap = serverData == nullptr ? 0 : serverData->limit / 1024 / 1024;

    int remainPercent = serverData != nullptr
            && serverData->limit > 0 ? static_cast<int>(serverData->remaining * 100 / serverData->limit) : 0;

    if (serverData->limit > 0)
    {
        stream1 << " of " << cap << " MB";

        QString status2;
        QTextStream stream2(&status2);

        stream2 << serverData->remaining / 1024 / 1024 << " MB left" << " (" << remainPercent << "%)";

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

    if (serverData->usedWarning > 0 && serverData->used > serverData->usedWarning &&
            serverData->usedLastSeen > serverData->lastChange) {


    }
}

void MainWindow::parseReply(QNetworkReply* pReply)
{
    QByteArray reply(pReply->readAll());
    pReply->deleteLater();

    QJsonDocument doc = QJsonDocument::fromJson(reply);
    QJsonArray arr = doc.array();
    QJsonObject obj;

    if (arr.size() > 0 && arr[0].toObject().contains("serial")) { // list reply
        QStringList captions;
        QStringList serials;

        for (int i = 0; i < arr.size(); i++) {
            obj = arr[i].toObject();
            captions.append(obj.value("caption").toString());
            serials.append(obj.value("serial").toString());
        }

        ListDialog *listDialog = new ListDialog(this, captions, serials);
        connect(listDialog, SIGNAL(selected(QString)), this, SLOT(listItemSelected(QString)));
        listDialog->setModal(true);
        listDialog->show();

        return;
    }

    ServerData *newData = nullptr;

    for (int i = 0; i < arr.size(); i++) {
         obj = arr[i].toObject();

         if (obj.value("simserial").toString() == sim) {
             newData = new ServerData;

             newData->caption = obj.value("simcaption").toString();

             long long current = obj.value("current").toString().toLongLong();
             long long floor = obj.value("floor").toString().toLongLong();

             newData->active = false;
             newData->lastChange = obj.value("lastchange").toString().toLongLong();
             newData->lastUpdate = obj.value("lastupdate").toString().toLongLong();
             newData->used = current - floor;

             if (newData->used < 0)
                 newData->used = 0;

            if (obj.value("haslimit").toString() == "1") {
                newData->limit = obj.value("limit").toString().toLongLong();
                newData->remaining = newData->limit - newData->used;
                if (newData->remaining < 0)
                        newData->remaining = 0;
            } else {
                newData->limit = 0;
                newData->remaining = 0;
            }

             newData->usedWarning = obj.value("hasusedwarning").toString() == "1" ?
                         obj.value("usedwarning").toString().toLongLong() : 0;
             newData->usedLastSeen = obj.value("usedlastseen").toString().toLongLong();

             newData->remainWarning = obj.value("hasremainwarning").toString() == "1" ?
                         obj.value("remainwarning").toString().toLongLong() : 0;
             newData->remainLastSeen = obj.value("remainlastseen").toString().toLongLong();

             QJsonArray slotlists = obj.value("slots").toArray();

             for (int j = 0; j < slotlists.size(); j++) {
                 QJsonArray list = slotlists.at(j).toArray();

                 for (int k = 0; k < list.size(); k++) {
                    QStringList data = list.at(k).toString().split(":");

                    newData->slotlists.at(j)->update(k, data[0].toLongLong(), data[1].toLongLong(), data[2].toLongLong());
                 }
             }
         }
    }

    dataReceived(newData);
}

void MainWindow::serverError(QString message)
{
    QMessageBox::warning(this, tr(APPTITLE), tr("The built-in server encountered a problem:\n")
                         + message);
    server.hasError = false;
    dataReceived(nullptr);
}

void MainWindow::timerEvent(QTimerEvent *event)
{
    if (event == nullptr ||  event->timerId() != timerId)
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

    if (serverData != nullptr && QDateTime::currentDateTime().secsTo(lastReception) * 1000 < -maxDelay) {
        serverData->active = false;
        paintTrayIcon();
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    settings->setValue("pos", pos());
    hide();
    event->ignore();
}

void MainWindow::on_radioButton_clicked()
{
    if (ui->radioButton->isChecked()) {
        ui->radioButton_2->setChecked(false);
        ui->lineEditURL->setEnabled(true);
        ui->pushButtonVisit->setEnabled(true);
        ui->pushButtonList->setEnabled(simIsFromPrefs);
        ui->lineEditSimId->setEnabled(true);
        ui->spinBoxPort->setEnabled(false);
    } else {
        ui->radioButton->setChecked(true);
    }

    settings->setValue("builtin", ui->radioButton_2->isChecked());
    server.close();
    timerEvent(nullptr);
}

void MainWindow::on_lineEditURL_editingFinished()
{
    settings->setValue("url", ui->lineEditURL->text());
}

void MainWindow::on_pushButtonVisit_clicked()
{
    QString urlStr(settings->value("url", DEFAULT_URL).toString());
    if (!urlStr.endsWith("/"))
        urlStr += "/";

    QString simId = settings->value("simserial", "").toString();

    if (simId.length() > 0)
        urlStr += "?serial=" + simId;

    QDesktopServices::openUrl(QUrl(urlStr));
}

void MainWindow::on_radioButton_2_clicked()
{
    if (ui->radioButton_2->isChecked()) {
        ui->radioButton->setChecked(false);
        ui->lineEditURL->setEnabled(false);
        ui->pushButtonVisit->setEnabled(false);
        ui->pushButtonList->setEnabled(false);
        ui->lineEditSimId->setEnabled(false);
        ui->spinBoxPort->setEnabled(true);
    } else {
        ui->radioButton_2->setChecked(true);
    }

    settings->setValue("builtin", ui->radioButton_2->isChecked());
}

void MainWindow::on_spinBoxPort_valueChanged(int arg1)
{
    server.close();
    settings->setValue("port", arg1);
}


void MainWindow::on_pushButtonSecret_clicked()
{
    SecretDialog *secretDialog = new SecretDialog(this, settings->value("secret", "").toString());
    connect(secretDialog, SIGNAL(secretChanged(QString)), this, SLOT(secretChanged(QString)));
    secretDialog->setModal(true);
    secretDialog->show();
}

void MainWindow::on_lineEditSimId_editingFinished()
{
    settings->setValue("simserial", ui->lineEditSimId->text());
    sim = ui->lineEditSimId->text();

    server.close();
}

void MainWindow::on_pushButtonList_clicked()
{
    QString urlStr(settings->value("url", DEFAULT_URL).toString());
    if (!urlStr.endsWith("/"))
        urlStr += "/";
    QUrl url = urlStr + "?list=1";

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject obj;
    obj["secret"] = settings->value("secret").toString();
    QJsonDocument doc(obj);

    QByteArray data = doc.toJson();

    networkAccessManager.post(request, data);
}


void MainWindow::on_spinBox_hide_valueChanged(int arg1)
{
    settings->setValue("hide", arg1);
}

void MainWindow::on_pushButtonClose_clicked()
{
    settings->setValue("pos", pos());
    hide();
}

