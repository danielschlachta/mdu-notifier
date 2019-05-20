#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QPainter>
#include <QCloseEvent>


#define APPTITLE "Mobile Data Usage"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle(tr(APPTITLE));

    settingsAction = new QAction(tr("&Settings..."), this);
    connect(settingsAction, &QAction::triggered, this, &QWidget::showNormal);

    quitAction = new QAction(tr("&Quit " APPTITLE), this);
    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);

    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(settingsAction);
    trayIconMenu->addAction(quitAction);

    trayIcon = new QSystemTrayIcon(this);
    updateTrayIcon(17, true);

    connect(trayIcon, &QSystemTrayIcon::messageClicked, this, &MainWindow::iconMessageClicked);

    trayIcon->setContextMenu(trayIconMenu);
    trayIcon->show();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::iconMessageClicked()
{
    QMessageBox::information(nullptr, tr("Systray"),
                             tr("Sorry, I already gave what help I could.\n"
                                "Maybe you should try asking a human?"));
}

void MainWindow::updateTrayIcon(int percent, bool)
{
    QPixmap pixmap(200, 200);
    pixmap.fill(Qt::transparent);

    QColor ltGreen(Qt::green);
    QColor ltGray(160, 160, 160);


    QPainter painter(&pixmap);
    //painter.setRenderHint(QPainter::Antialiasing);

    QBrush brush;
    //brush->setColor(QColor(255,0,255,128));
    brush.setStyle(Qt::SolidPattern);

    QRectF outRect(0.0, 20.0, 160.0, 160.0);
    int startAngle = 90 * 16;

    painter.setPen(ltGray);
    brush.setColor(ltGray);
    painter.setBrush(brush);

    painter.drawPie(outRect, startAngle, 360 * 16);

    if (percent > 0)
    {
        int spanAngle = -percent * 360 / 100 * 16;

        brush.setColor(ltGreen);
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

void MainWindow::closeEvent(QCloseEvent *event)
{
    hide();
    event->ignore();
}
