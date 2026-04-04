#pragma once

#include "connection.h"
#include "BackendResponse.h"
#include "BackendQueryResponse.h"
#include "matches.h"

using namespace std;

class LogHistoric;

class LogMatch {
private:
    Connection& connection_;
    LogHistoric* logHistoric;

public:
    LogMatch(Connection& dbConnection);

    void setLogHistoric(LogHistoric* historic);
    BackendResponse insert(int tournamentId, int teamA, int teamB);
    BackendQueryResponse<Match> listByTournament(int tournamentId);
    BackendQueryResponse<Match> listByPhase(int id, string phase);
    BackendResponse update(int id, string phase, int round, string status, int winnerId, string result);
    BackendResponse eliminar(int id);
};
