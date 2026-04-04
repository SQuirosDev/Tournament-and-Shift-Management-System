#pragma once

#include <string>
#include "connection.h"
#include "backendResponse.h"
#include "backendQueryResponse.h"

using namespace std;

class LogHistoric;

class LogTournament {
private:
    Connection& connection_;
    LogHistoric* logHistoric;

public:
    LogTournament(Connection& dbConnection);

    void setLogHistoric(LogHistoric* historic);
    BackendResponse insert(string name);
    BackendQueryResponse<Tournament> list();
    BackendQueryResponse<Tournament> obtain(int id);
    BackendResponse updateName(int id, string newName);
    BackendResponse updatePhase(int id, string phase);
    BackendResponse eliminar(int id);
};
