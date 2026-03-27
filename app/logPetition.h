#pragma once

#include "connection.h"
#include "queue.h"
#include "backendResponse.h"
#include "backendQueryResponse.h"
#include "Petitions.h"

class LogPetitions {
public:
    LogPetitions(Connection& dbConnection);

    BackendResponse registerPetition(string requesterName, string type, string description);
    BackendResponse updatePetition(int id, string responseText);
    BackendQueryResponse<Petition> peekNextPetition();
    BackendQueryResponse<Petition> listPendingPetitions();
    int pendingCount();

private:
    Connection& connection_;
};