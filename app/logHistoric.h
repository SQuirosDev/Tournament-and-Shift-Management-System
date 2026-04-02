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

class LogHistoric {
private:
    Connection& connection;
    LogPlayer* logPlayer;
    LogPetition* logPetition;
    LogTeam* logTeam;

public:
    LogHistoric(Connection& dbConnection);

    void setLogPlayer(LogPlayer* player);
    void setLogPetition(LogPetition* petition);
    void setLogTeam(LogTeam* team);
    BackendResponse insert(Historic h);
    BackendResponse undo();

private:
    BackendResponse undoPlayer(Historic& h);
    BackendResponse undoPetition(Historic& h);
    BackendResponse undoTeam(Historic& h);
};
