#pragma once

#include "stackHistoric.h"

NodeHistoric* topNode;

StackHistoric::StackHistoric() : topNode(nullptr) {}

StackHistoric::~StackHistoric() {
    while (!isEmpty()) {
        pop();
    }
}

void StackHistoric::push(Historic& h) {
    NodeHistoric* newNode = new NodeHistoric();
    newNode->data = h;
    newNode->next = topNode;
    topNode = newNode;
}

void StackHistoric::pop() {
    if (isEmpty()) return;

    NodeHistoric* temp = topNode;
    topNode = topNode->next;
    delete temp;
}

Historic StackHistoric::top() {
    if (isEmpty()) {
        qDebug() << "Stack vacía";
    }
    return topNode->data;
}

bool StackHistoric::isEmpty() {
    return topNode == nullptr;
}
