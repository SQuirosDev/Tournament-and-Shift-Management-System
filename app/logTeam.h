#pragma once

#include "connection.h"
#include "backendResponse.h"
#include "backendQueryResponse.h"

using namespace std;

class LogHistoric;

class LogTeam {
private:
    Connection& connection_;
    LogHistoric* logHistoric;

public:
    LogTeam(Connection& dbConnection);

    void setLogHistoric(LogHistoric* historic);
    BackendResponse insert(string name, int tournamentId);
    BackendQueryResponse<Team> list();
    BackendResponse generate(int tournamentId);
    BackendResponse update(int id, string newName);
    BackendResponse eliminar(int id);
};
