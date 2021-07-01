#ifndef SLOTLIST_H
#define SLOTLIST_H

#include <QVector>

struct Slot
{
    long startTime = 0;
    long rxBytes = 0;
    long txBytes = 0;
};

class SlotList : public QVector<Slot>
{
public:
    int listId;
    int slotInterval;
    int labelCount;
    int currentIndex = 0;
    long currentTime = 0;

    SlotList(int listId, int slotCount, int slotInterval, int labelCount);

    long getMaxBytes();
    Slot *get(int index);
    void setCurrentTime(long currentTime);
    void update(int slotId, long startTime, long rxBytes, long txBytes);
};

#endif // SLOTLIST_H
