#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QPainter>
#include <QCloseEvent>
#include <QTextCodec>

#define APPTITLE "Mobile Data Usage"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle(tr(APPTITLE));

    ageThreshold = 20;
    lastReception.start();

    settingsAction = new QAction(tr("&Settings..."), this);
    connect(settingsAction, &QAction::triggered, this, &QWidget::showNormal);

    quitAction = new QAction(tr("&Quit " APPTITLE), this);
    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);

    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(settingsAction);
    trayIconMenu->addAction(quitAction);

    trayIcon = new QSystemTrayIcon(this);
    paintTrayIcon();

    connect(trayIcon, &QSystemTrayIcon::messageClicked, this, &MainWindow::iconMessageClicked);

    trayIcon->setContextMenu(trayIconMenu);
    trayIcon->show();

    timerId = startTimer(5000);
}

MainWindow::~MainWindow()
{
    killTimer(timerId);
    delete ui;
}

void MainWindow::iconMessageClicked()
{
    QMessageBox::information(nullptr, tr("Systray"),
                             tr("Sorry, I already gave what help I could.\n"
                                "Maybe you should try asking a human?"));
}

void MainWindow::paintTrayIcon()
{
    bool active = age <= ageThreshold && lastReception.elapsed() < ageThreshold * 1000;

    QPixmap pixmap(200, 200);
    pixmap.fill(Qt::transparent);

    QColor ltGreen(Qt::green);
    QColor ltGray(160, 160, 160);
    QColor dkGray(100, 100, 100);

    QPainter painter(&pixmap);
    //painter.setRenderHint(QPainter::Antialiasing);

    QBrush brush;
    brush.setStyle(Qt::SolidPattern);
    QRectF outRect(0.0, 20.0, 160.0, 160.0);
    int startAngle = 90 * 16;

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
    painter.drawPie(outRect, startAngle, 360 * 16);

    if (percent > 0 && active)
    {
        int spanAngle = -percent * 360 / 100 * 16;

        painter.setPen(ltGreen);
        brush.setColor(ltGreen);
        painter.setBrush(brush);
        painter.drawPie(outRect, startAngle, spanAngle);
    }

    double inRad = 50.0;

    QRectF inRect(outRect.left() + inRad, outRect.top() + inRad,
                  outRect.width() - inRad * 2, outRect.height() - inRad * 2);

    brush.setColor(QColor(70, 70, 70));
    painter.setBrush(brush);
    painter.drawPie(inRect, startAngle, 360 * 16);

    QIcon icon(pixmap);
    trayIcon->setIcon(icon);
}

void MainWindow::parseReply()
{
    QStringList data = tr(pFileDownloader->reply.data()).split("/");

    if (data.count() == 3)
    {
        age = data.value(0).toLong();
        usedBytes = data.value(1).toLongLong();
        capBytes = data.value(2).toLongLong();

        percent = capBytes > 0 ? static_cast<int>(usedBytes * 100 / capBytes) : 0;

        lastReception.start();
    }
}

void MainWindow::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == timerId)
    {
        paintTrayIcon();

        QUrl url("http://localhost/mdu/mdu-notifier.php");

        pFileDownloader = new FileDownloader(url, this);
        connect(pFileDownloader, SIGNAL(downloaded()), this, SLOT(parseReply()));
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    hide();
    event->ignore();
}
