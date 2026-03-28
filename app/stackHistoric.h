#pragma once

#include <QDebug>
#include "nodeHistoric.h"

class StackHistoric {
private:
    NodeHistoric* topNode;

public:
    StackHistoric();
    ~StackHistoric();
    void push(Historic& h);
    void pop();
    Historic top();
    bool isEmpty();
};
