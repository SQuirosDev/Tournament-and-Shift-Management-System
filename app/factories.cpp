#include "factories.h"

BackendResponse dbResponseFactory(DbResponse dbRes)
{
    BackendResponse backendRes;
    backendRes.id = dbRes.id;
    backendRes.code = dbRes.code;
    backendRes.message = dbRes.message;
    return backendRes;
}