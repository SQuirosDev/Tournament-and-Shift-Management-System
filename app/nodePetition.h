#pragma once
#include "petitions.h"

struct NodePetition {
    Petition data;
    NodePetition* next;
};
