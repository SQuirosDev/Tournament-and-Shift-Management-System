#pragma once

#include <string>
#include <vector>

using namespace std;

template <typename T>
struct BackendQueryResponse {
    vector<T> data;
    int code = 0;
    string message;
};