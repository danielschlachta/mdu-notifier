#include "showtraffic.h"
#include "ui_showtraffic.h"

ShowTraffic::ShowTraffic(QWidget *parent, QSettings *settings) :
    QMainWindow(parent),
    ui(new Ui::ShowTraffic)
{
    ui->setupUi(this);
    ui->toolBox->addItem(&bg1, "Last 90 seconds");
    ui->toolBox->addItem(&bg2, "Last 120 minutes");
    ui->toolBox->addItem(&bg3, "Last 12 hours");

    this->setWindowTitle("Traffic - " + this->windowTitle());

    this->settings = settings;
}

ShowTraffic::~ShowTraffic()
{
    delete ui;
}

void ShowTraffic::closeEvent(QCloseEvent *event)
{
    settings->setValue("trafficpos", pos());
    settings->setValue("trafficsize", size());
    hide();
    event->ignore();
}

void ShowTraffic::updateData(ServerData &serverData)
{
    ui->label->setText(serverData.caption);

    bg1.slotList = serverData.slotlists.data()[0];
    bg1.repaint();

    bg2.slotList = serverData.slotlists.data()[1];
    bg2.repaint();

    bg3.slotList = serverData.slotlists.data()[2];
    bg3.repaint();
}
