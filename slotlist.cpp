#include "slotlist.h"

SlotList::SlotList(int listId, int slotCount, int slotInterval, int labelCount) : QVector(slotCount)
{
    this->listId = listId;
    this->slotInterval = slotInterval;
    this->labelCount = labelCount;
}

long SlotList::getMaxBytes()
{
    long max = 0;

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

void SlotList::setCurrentTime(long currentTime)
{
    this->currentTime = currentTime - (currentTime % slotInterval);
    this->currentIndex = (int) (this->currentTime / slotInterval) % size();
}

void SlotList::update(int slotId, long startTime, long rxBytes, long txBytes)
{
    this->data()[slotId].startTime = startTime;
    this->data()[slotId].rxBytes = rxBytes;
    this->data()[slotId].txBytes = txBytes;

    if (startTime > this->currentTime)
        setCurrentTime(startTime);
}
