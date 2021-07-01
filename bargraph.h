#ifndef BARGRAPH_H
#define BARGRAPH_H

#include <QWidget>
#include <QPaintEvent>
#include <QPainter>
#include <QFont>
#include <QFontMetrics>
#include <QColor>

#include "slotlist.h"

class BarGraph : public QWidget
{
    Q_OBJECT
public:
    SlotList *slotList;

    explicit BarGraph(QWidget *parent = nullptr);
    ~BarGraph();

    void paintEvent(QPaintEvent *event);

    void paint(QPainter &painter, QPaintEvent *event);

private:

    QFont *font;
    QFontMetrics *fontMetric;
    QColor green = QColor(0x60, 0x9F, 0x99);
    QColor blue = QColor(0x51, 0x61, 0x85);

    int lineStep;

    void drawVertLine(QPainter &painter, int x);
    void drawHorizLine(QPainter &painter, int y, int width);

signals:

};

#endif // BARGRAPH_H
