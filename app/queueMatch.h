#pragma once

#include <QDebug>
#include "nodeMatch.h"
#include "matches.h"

class QueueMatch {
private:
    NodeMatch* frontNode;
    NodeMatch* rearNode;

public:
    QueueMatch();
    ~QueueMatch();

    void enqueue(Match& match);
    void dequeue();
    Match front();
    bool isEmpty();
};
