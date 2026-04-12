#include "queueMatch.h"

QueueMatch::QueueMatch() : frontNode(nullptr), rearNode(nullptr) {}

QueueMatch::~QueueMatch() {
    while (!isEmpty()) {
        dequeue();
    }
}

void QueueMatch::enqueue(Match& match) {
    NodeMatch* newNode = new NodeMatch();
    newNode->data = match;
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

void QueueMatch::dequeue() {
    if (isEmpty()) {
        return;
    }

    NodeMatch* temp = frontNode;
    frontNode = frontNode->next;

    if (frontNode == nullptr) {
        rearNode = nullptr;
    }

    delete temp;
}

Match QueueMatch::front() {
    if (isEmpty()) {
        qDebug() << "Queue vacia";
    }
    return frontNode->data;
}

bool QueueMatch::isEmpty() {
    return frontNode == nullptr;
}
