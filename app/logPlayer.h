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

    BackendResponse registerPlayer(int teamId, string playerName);
    BackendQueryResponse<Player> listPlayersByTeam(int teamId);
    BackendQueryResponse<Player> getPlayerById(int playerId, Player& outputRow);
    BackendResponse updatePlayerName(int playerId, string newName);
    BackendResponse removePlayer(int playerId);

private:
    Connection& connection_;
};