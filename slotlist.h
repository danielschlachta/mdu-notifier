#ifndef SLOTLIST_H
#define SLOTLIST_H

#include <QVector>

struct Slot
{
    long long startTime = 0;
    long long rxBytes = 0;
    long long txBytes = 0;
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
    void setCurrentTime(long long currentTime);
    void update(int slotId, long long startTime, long long rxBytes, long long txBytes);
};

#endif // SLOTLIST_H
