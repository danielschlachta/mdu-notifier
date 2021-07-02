#include <QPainterPath>
#include <QDateTime>

#include "bargraph.h"

BarGraph::BarGraph(QWidget *parent) : QWidget(parent)
{
    font = new QFont("sans-serif", 12);
    fontMetric = new QFontMetrics(*font);
    lineStep = fontMetric->height() / 3;
    green.setAlpha(0xC0);
    blue.setAlpha(0x80);
}

BarGraph::~BarGraph()
{
    delete fontMetric;
    delete font;
}

void BarGraph::paintEvent(QPaintEvent *event)
{
    if (slotList != nullptr) {
        QPainter painter;
        painter.begin(this);
        painter.setRenderHint(QPainter::Antialiasing);
        paint(painter, event);
        painter.end();
    }
}


void BarGraph::drawVertLine(QPainter &painter, int x)
{
    int i = lineStep * 2;

    while (i < height()) {
        painter.drawLine(x, i - lineStep * 2, x, i - lineStep);
        i = i + lineStep * 2;
    }

    painter.drawLine(x, i - lineStep * 2, x, height() - 1);
}

void BarGraph::drawHorizLine(QPainter &painter, int y, int width)
{
    int i = lineStep * 2;

    while (i - lineStep <= width) {
        painter.drawLine(i - lineStep * 2, y, i - lineStep, y);
        i = i + lineStep * 2;
    }

    painter.drawLine(i - lineStep * 2, y, width - 1, y);
}

void BarGraph::paint(QPainter &painter, QPaintEvent *event)
{
    if (event == nullptr)
        return;

    int marginTop = 20;
    int marginBottom = 16;

    int plotHeight = height() - marginBottom - marginTop - 2;
    int colWidth = width() / slotList->size();
    int colStep = colWidth / 10;

    long maxData = slotList->getMaxBytes();

    QString dimString = "Byte";
    long dim = 1;

    if (maxData > 1024) {
        dimString = "Kilobyte";
        dim = 1024;
    }

    if (maxData > 1024 * 1024) {
        dimString = "Megabyte";
        dim = 1024 * 1024;
    }

    if (maxData > 1024 * 1024 * 1024) {
        dimString = "Gigabyte";
        dim = 1024 * 1024 * 1024;
    }

    int cap = dim < 1000 ? 10 : 2;
    while (cap * dim < maxData)
        cap++;

    double factor = 1.0 * plotHeight / (cap * dim);

    for (int i = 0; i < slotList->labelCount; i++) {

        int posX = i * colWidth * slotList->size() /
                slotList->labelCount + 1;

        drawVertLine(painter, posX);

        long labelInterval = slotList->size() / slotList->labelCount * slotList->slotInterval;
        long long millis = slotList->currentTime - (slotList->size() - 1) * slotList->slotInterval + i * labelInterval;
        QDateTime time = QDateTime::fromTime_t(static_cast<uint>(millis / 1000));
        QString label = time.toString("hh:mm:ss");

        if (slotList->labelCount > 4)
            label = time.toString("hh:mm");

        painter.drawText(posX + 4, height(), label);
    }

    int lines = 4; // excluding top

    while ((lines > 1) &&
        static_cast<int>(1.8 * fontMetric->height()) > static_cast<int>(dim * (cap / lines) * factor))
        lines--;

    for (int l = lines, frac = 0; l > 2; l--) {
        int maxfrac = 0;

        for (int i = 1; i < l; i++)
            if ((cap / l * i) % 10 > maxfrac)
                maxfrac = (cap / l * i) % 10;

        if (frac == 0 || maxfrac < frac) {
            lines = l;
            frac = maxfrac;
        }
    }

    for (int i = cap, step = cap / lines; i > 0; i -= step > 0 ? step : 1) {
        int y = height() - dim * i * factor - marginBottom - 1;

        drawHorizLine(painter, y, slotList->size() * colWidth);
        painter.drawText(7, y - 7, QString::number(i) + " " + dimString + (i > 1 ? "s" : ""));
    }

    painter.drawLine(1, 1, 1, height() - marginBottom + 1);

    painter.drawLine(1, height() - marginBottom + 1, slotList->size() * colWidth, height() - marginBottom + 1);


    for (int i = 0; i < slotList->size(); i++) {
        Slot *slot = slotList->get(i);

        int ofs = i > 0 ? (i < slotList->size() / 4 * 3 ? 1 : 0) : -1;

        int txData = static_cast<int>(slot->txBytes * factor);

        if (txData > 0) {
            int left = colWidth * i + colStep * 3 + 1 + ofs;
            int right = colWidth * (i + 1) - colStep + ofs;
            int top = height() - txData - marginBottom;
            int bottom = height() - marginBottom;

            QRect txRect(left, top, right - left, bottom - top);
            painter.fillRect(txRect, blue);
        }

        int rxData = static_cast<int>(slot->rxBytes * factor);

        if (rxData > 0) {
            int left = colWidth * i + colStep + 1 + ofs;
            int right = colWidth * (i + 1) - colStep * 3 + ofs;
            int top = height() - rxData - marginBottom;
            int bottom = height() - marginBottom;

            QRect rxRect(left, top, right - left, bottom - top);

            QBrush brush;
            painter.fillRect(rxRect, green);
        }
    }
}


