#pragma once

#include <string>
#include <vector>
#include "connection.h"
#include "backendResponse.h"
#include "backendQueryResponse.h"
#include "Players.h"

using namespace std;

class LogPlayer {
public:
    LogPlayer(Connection& dbConnection);

    BackendResponse insert(int teamId, string playerName);
    BackendQueryResponse<Player> listByTeam(int teamId);
    BackendQueryResponse<Player> getById(int playerId, Player& outputRow);
    BackendResponse update(int playerId, string newName);
    BackendResponse eliminar(int playerId);

private:
    Connection& connection_;
};