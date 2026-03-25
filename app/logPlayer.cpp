#include "logPlayer.h"
#include "validations.h"

LogPlayer::LogPlayer(Connection& dbConnection) : connection_(dbConnection) {}

DbResponse LogPlayer::registerPlayer(int teamId, const string& playerName) {
    Validations Validation;
    if (teamId <= 0) {
        return { -1, 400, "El ID del equipo no es válido." };
    }
    if (!Validation.isValidName(playerName)) {
        return { -1, 400, "El nombre del jugador no puede estar vacío." };
    }
    return connection_.insertPlayer(playerName, teamId);
}

DbResponse LogPlayer::listPlayersByTeam(int teamId, vector<Player>& outputList) {
    if (teamId <= 0) {
        return { -1, 400, "El ID del equipo no es válido." };
    }
    return connection_.listPlayersByTeam(teamId, outputList);
}

DbResponse LogPlayer::getPlayerById(int playerId, Player& outputRow) {
    if (playerId <= 0) {
        return { -1, 400, "El ID del jugador no es válido." };
    }
    return connection_.obtainPlayerById(playerId, outputRow);
}

DbResponse LogPlayer::updatePlayerName(int playerId, const string& newName) {
    Validations Validation;
    if (playerId <= 0) {
        return { -1, 400, "El ID del jugador no es válido." };
    }
    if (!Validation.isValidName(newName)) {
        return { -1, 400, "El nombre del jugador no puede estar vacío." };
    }
    return connection_.updatePlayer(playerId, newName);
}

DbResponse LogPlayer::removePlayer(int playerId) {
    if (playerId <= 0) {
        return { -1, 400, "El ID del jugador no es válido." };
    }
    return connection_.deletePlayer(playerId);
}