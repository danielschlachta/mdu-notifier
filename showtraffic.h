#ifndef SHOWTRAFFIC_H
#define SHOWTRAFFIC_H

#include <QMainWindow>
#include <QSettings>
#include <QJsonArray>
#include <QCloseEvent>

#include "server.h"
#include "bargraph.h"

namespace Ui {
class ShowTraffic;
}

class ShowTraffic : public QMainWindow
{
    Q_OBJECT

public:
    explicit ShowTraffic(QWidget *parent = nullptr, QSettings *settings = nullptr);
    ~ShowTraffic();

    void updateData(ServerData &serverData);

private:
    void closeEvent(QCloseEvent *event);

    QSettings *settings;

    BarGraph bg1;
    BarGraph bg2;
    BarGraph bg3;

    Ui::ShowTraffic *ui;
};

#endif // SHOWTRAFFIC_H
