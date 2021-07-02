#include "slotlist.h"

SlotList::SlotList(int listId, int slotCount, int slotInterval, int labelCount) : QVector(slotCount)
{
    this->listId = listId;
    this->slotInterval = slotInterval;
    this->labelCount = labelCount;
}

long long SlotList::getMaxBytes()
{
    long long max = 0;

     for (int i = 0; i < size(); i++) {
         Slot *slot = &data()[i];

         if (slot->rxBytes > max)
             max = slot->rxBytes;

         if (slot->txBytes > max)
             max = slot->txBytes;
     }

     return max;
}

Slot *SlotList::get(int index)
{
    if (currentTime == 0)
        return &this->data()[(currentIndex + index) % size()];

    return &this->data()[(currentIndex + index + 1) % this->size()];
}

void SlotList::setCurrentTime(long long currentTime)
{
    this->currentTime = currentTime - (currentTime % slotInterval);
    this->currentIndex = static_cast<int>(this->currentTime / slotInterval) % size();
}

void SlotList::update(int slotId, long long startTime, long long rxBytes, long long txBytes)
{
    this->data()[slotId].startTime = startTime;
    this->data()[slotId].rxBytes = rxBytes;
    this->data()[slotId].txBytes = txBytes;

    if (startTime > this->currentTime)
        setCurrentTime(startTime);
}
