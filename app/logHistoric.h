#pragma once

#include "connection.h"
#include "backendResponse.h"
#include "dbResponse.h"
#include "dbQueryResponse.h"
#include "Historics.h"
#include "json.hpp"

using namespace std;
using json = nlohmann::json;

class LogPlayer;
class LogPetition;
class LogTeam;
class LogTournament;
class LogMatch;

class LogHistoric {
private:
    Connection& connection;
    LogPlayer* logPlayer;
    LogPetition* logPetition;
    LogTeam* logTeam;
    LogTournament* logTournament;
    LogMatch* logMatch;

public:
    LogHistoric(Connection& dbConnection);

    void setLogPlayer(LogPlayer* player);
    void setLogPetition(LogPetition* petition);
    void setLogTeam(LogTeam* team);
    void setLogTournament(LogTournament* tournament);
    void setLogMatch(LogMatch* match);
    BackendResponse insert(Historic h);
    BackendResponse undo();

private:
    BackendResponse undoPlayer(Historic& h);
    BackendResponse undoPetition(Historic& h);
    BackendResponse undoTeam(Historic& h);
    BackendResponse undoTournament(Historic& h);
    BackendResponse undoMatch(Historic& h);
};
