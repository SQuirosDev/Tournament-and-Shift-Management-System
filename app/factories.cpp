#include "factories.h"

BackendResponse dbResponseFactory(DbResponse dbRes)
{
    BackendResponse backendRes;
    backendRes.id = dbRes.id;
    backendRes.code = dbRes.code;
    backendRes.message = dbRes.message;
    return backendRes;
}

Historic historicFactory(string action, string entity, int id, string previousData, string newData) {
    Historic historic;
    historic.actionType = action;
    historic.entityType = entity;
    historic.recordId = id;
    historic.previousData = previousData;
    historic.newData = newData;
    historic.stackPosition = 0;
    return historic;
}
