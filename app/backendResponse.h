#pragma once

#include <string>

using namespace std;

struct BackendResponse {
    int id = -1;
    int code = 0;
    string message;
};