#pragma once

#include <string>
#include "connection.h"
#include "BackendResponse.h"

using namespace std;

class LogHistoric;


class LogGame {
private:
    Connection& connection_;

public:
    LogGame(Connection& dbConnection);

    // GRUPOS
    BackendResponse generateGroupMatches(int tournamentId);
    BackendResponse playGroupMatches(int tournamentId);

    // SEMIFINALES
    BackendResponse generateSemiMatches(int tournamentId);
    BackendResponse playSemiMatches(int tournamentId);

    // FINAL
    BackendResponse generateFinalMatch(int tournamentId);
    BackendResponse playFinalMatch(int tournamentId);

}; 
