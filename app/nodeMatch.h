#pragma once
#include "matches.h"

struct NodeMatch {
    Match data;
    NodeMatch* next;
};
