#pragma once

#include <string>
#include <vector>
#include "connection.h"

using namespace std;

class LogPlayer {
public:
    LogPlayer(Connection& dbConnection);

    DbResponse registerPlayer(int teamId, const string& playerName);
    DbResponse listPlayersByTeam(int teamId, vector<Player>& outputList);
    DbResponse getPlayerById(int playerId, Player& outputRow);
    DbResponse updatePlayerName(int playerId, const string& newName);
    DbResponse removePlayer(int playerId);

private:
    Connection& connection_;
};