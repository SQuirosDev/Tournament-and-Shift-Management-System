#pragma once
#include <QDebug>
#include "nodePetition.h"
#include "petitions.h"

class QueuePetition {
private:
    NodePetition* frontNode;
    NodePetition* rearNode;

public:
    QueuePetition();
    ~QueuePetition();

    void enqueue(Petition& petition);
    void dequeue();
    Petition front();
    bool isEmpty();
};
