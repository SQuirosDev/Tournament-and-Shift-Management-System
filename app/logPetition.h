#pragma once

#include "connection.h"
#include "queue.h"
#include "backendResponse.h"
#include "backendQueryResponse.h"
#include "Petitions.h"

using namespace std;

class LogHistoric;

class LogPetition {
public:
    LogPetition(Connection& dbConnection);

    void setLogHistoric(LogHistoric* historic);
    BackendResponse insert(string requesterName, string type, string description);
    BackendResponse update(int id, string responseText);
    BackendQueryResponse<Petition> peekNextPetition();
    BackendQueryResponse<Petition> listPendingPetitions();
    // hacer funcion
    BackendResponse eliminar(int id);
    int pendingCount();

private:
    Connection& connection_;
    LogHistoric* logHistoric;
};