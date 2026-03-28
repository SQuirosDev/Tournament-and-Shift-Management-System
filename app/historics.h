#pragma once

#include <string>

using namespace std;

struct Historic {
    int id = 0;
    string actionType;
    string entityType;
    int recordId = 0;
    string previousData;
    string newData;
    int stackPosition = 0;
};
