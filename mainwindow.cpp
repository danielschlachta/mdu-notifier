#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QCloseEvent>
#include <QWindow>

#define APPTITLE "Mobile Data Usage"
#define DEFAULT_URL "http://localhost/mdu"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    settings = new QSettings("mdu-notifier", "mdu-notifier");

    move(settings->value("pos", QPoint(200, 200)).toPoint());

    ui->setupUi(this);
    ui->lineEditURL->setText(settings->value("url", tr(DEFAULT_URL)).toString());
    ui->spinBox_show->setValue(settings->value("show", 2).toInt());
    ui->spinBox_hide->setValue(settings->value("hide", 10).toInt());
    ui->checkBoxSuppress->setCheckState(settings->value("captime", 0).toLongLong() > 0 ?
                                            Qt::Checked : Qt::Unchecked);

    status1Action = new QAction(tr("..."), this);
    status1Action->setEnabled(false);
    status2Action = new QAction(tr("..."), this);
    status2Action->setEnabled(false);

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
            SIGNAL (finished(QNetworkReply*)), this, SLOT (parseReply(QNetworkReply*)));

    lastReception.start();
    timerId = startTimer(1000);
}

MainWindow::~MainWindow()
{
    killTimer(timerId);
    delete ui;
    delete settings;
}

long long MainWindow::inMegabytes(long long mb)
{
    return displayMetric ? mb / 1000 / 1000 : mb / 1024 / 1024;
}

void MainWindow::iconMessageClicked()
{
    settings->setValue("captime", static_cast<long long>(capTime));
    ui->checkBoxSuppress->setCheckState(Qt::Checked);
}

void MainWindow::showWindow()
{
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

    if (active)
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

    long long percent = capBytes > 0 ? usedBytes * 100 / capBytes : 0;

    if (percent > 0 && active)
    {
        painter.setPen(ltBlue);
        brush.setColor(ltBlue);
        painter.setBrush(brush);
        painter.drawPie(outRect, 90 * 16, static_cast<int>(-percent * 360 / 100 * 16));
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

    if (active && capBytes > 0
            && (capBytes - usedBytes) * 100 < capBytes * warn
            && settings->value("captime", 0).toInt() < capTime
            && show > 0
            && (messageShown.elapsed() == 0 || messageShown.elapsed() >= show * 1000 * 60))
    {
        QString msg;
        QTextStream stream(&msg);
        stream << inMegabytes(usedBytes) << " MB remaining ("
               << (capBytes - usedBytes) * 100 / capBytes << "%)" << endl << endl
               << "This message will appear again in " << show << " minutes unless you click on it.";

        trayIcon->showMessage(tr("Mobile data is running low"), msg, icon,
                              settings->value("hide", 0).toInt() * 1000);

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
        recvDelay = data.value(0).toLong();
        capTime = data.value(1).toLong();
        interval = data.value(2).toInt();
        warn = data.value(3).toInt();
        displayMetric = data.value(4).toInt() == 0;
        usedBytes = data.value(5).toLongLong();
        capBytes = data.value(6).toLongLong();

        active = recvDelay < interval + maxDelay && lastReception.elapsed() < maxTransmitAge * 1000;

        lastReception.restart();
    }
}

void MainWindow::timerEvent(QTimerEvent *event)
{
    settings->setValue("pos", pos());

    if (event->timerId() == timerId && lastReception.elapsed() >= transmitInterval)
    {
        QUrl url = settings->value("url", DEFAULT_URL).toString() + "/mdu-notifier.php";
        QNetworkRequest request(url);
        networkAccessManager.get(request);
    }

    active = recvDelay < interval + maxDelay && lastReception.elapsed() < maxTransmitAge * 1000;

    paintTrayIcon();

    QString status1;
    QTextStream stream1(&status1);
    stream1 << inMegabytes(usedBytes) << " MB of " << inMegabytes(capBytes) << " MB";

    QString status2;
    QTextStream stream2(&status2);

    stream2 << (capBytes > usedBytes ? inMegabytes(capBytes)  - inMegabytes(usedBytes) : 0) << " MB left";
    if (capBytes > 0)
        stream2 << " (" << (capBytes - usedBytes) * 100 / capBytes << "%)";

    status1Action->setVisible(true);
    status1Action->setText(status1);
    status2Action->setVisible(true);
    status2Action->setText(status2);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
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
        settings->setValue("captime", static_cast<long long>(capTime));
    else
        settings->setValue("captime", 0);
}

void MainWindow::on_pushButtonClose_clicked()
{
    hide();
}

