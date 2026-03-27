#pragma once

#include <string>

using namespace std;

struct DbResponse {
    int id = -1;
    int code = 0;
    string message;
};
