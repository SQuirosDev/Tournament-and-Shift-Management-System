#pragma once

#include "Historics.h"

struct NodeHistoric {
    Historic data;
    NodeHistoric* next;
};