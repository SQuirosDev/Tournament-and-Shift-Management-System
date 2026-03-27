#pragma once

#include <string>
#include "dbResponse.h"
#include "dbQueryResponse.h"
#include "backendResponse.h"
#include "backendQueryResponse.h"

using namespace std;

BackendResponse dbResponseFactory(DbResponse dbRes);

// los templates deben estar visibles en compilación para que el compilador genere el código para cada tipo usado.
template <typename T>
BackendQueryResponse<T> dbQueryResponseFactory(DBQueryResponse<T> dbRes)
{
    BackendQueryResponse<T> backendRes;
    backendRes.data = dbRes.data;
    backendRes.code = dbRes.code;
    backendRes.message = dbRes.message;
    return backendRes;
}
