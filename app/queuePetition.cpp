#include "queuePetition.h"

QueuePetition::QueuePetition() : frontNode(nullptr), rearNode(nullptr) {}

QueuePetition::~QueuePetition() {
    while (!isEmpty()) {
        dequeue();
    }
}

void QueuePetition::enqueue(Petition& petition) {
    NodePetition* newNode = new NodePetition();
    newNode->data = petition;
    newNode->next = nullptr;

    if (rearNode == nullptr) {
        frontNode = newNode;
        rearNode = newNode;
    }
    else {
        rearNode->next = newNode;
        rearNode = newNode;
    }
}

void QueuePetition::dequeue() {
    if (isEmpty()) {
        return;
    }

    NodePetition* temp = frontNode;
    frontNode = frontNode->next;

    if (frontNode == nullptr) {
        rearNode = nullptr;
    }

    delete temp;
}

Petition QueuePetition::front() {
    if (isEmpty()) {
        qDebug() << "Queue vacia";
    }

    return frontNode->data;
}

bool QueuePetition::isEmpty() {
    return frontNode == nullptr;
}
