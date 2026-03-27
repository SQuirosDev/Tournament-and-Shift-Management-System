#pragma once

#include <string>
#include <vector>

using namespace std;

template <typename T>
struct DBQueryResponse {
    vector<T> data;
    int code = 0;
    string message;
};