#pragma once

#include <string>
#include <vector>
#include "Historics.h"
#include "backendResponse.h"
#include "dbResponse.h"
#include "dbQueryResponse.h"
#include "connection.h"
#include "logPlayer.h"
#include "logPetition.h"
#include "codes.h"
#include "json.hpp"
#include "factories.h"
#include "stackHistoric.h"

using namespace std;
using json = nlohmann::json;

class LogHistoric {
private:
    Connection& connection;
    LogPlayer& logPlayer;
    LogPetition& logPetition;

public:
    LogHistoric(Connection& dbConnection, LogPlayer& player, LogPetition& petition);

    BackendResponse insert(Historic& h);
    BackendResponse undo();

private:
    BackendResponse undoPlayer(Historic& h);
    BackendResponse undoPetition(Historic& h);
};
