#include "logPlayer.h"
#include "validations.h"
#include "factories.h"

LogPlayer::LogPlayer(Connection& dbConnection) : connection_(dbConnection) {}

BackendResponse LogPlayer::insert(int teamId, string playerName) {

    if (teamId <= 0) {
        return { -1, 400, "El ID del equipo no es válido." };
    }

    if (!isValidName(playerName)) {
        return { 0, 400, "El nombre del jugador no puede estar vacío." };
    }

    return dbResponseFactory(connection_.insertPlayer(playerName, teamId));
}

BackendQueryResponse<Player> LogPlayer::listByTeam(int teamId) {

    if (teamId <= 0) {
        return { {}, 400, "El ID del equipo no es válido." };
    }

    return dbQueryResponseFactory<Player>(connection_.listPlayersByTeam(teamId));
}

BackendQueryResponse<Player> LogPlayer::getById(int playerId, Player& outputRow) {

    if (playerId <= 0) {
        return { {}, 400, "El ID del jugador no es válido." };
    }

    return dbQueryResponseFactory<Player>(connection_.obtainPlayerById(playerId));
}

BackendResponse LogPlayer::update(int playerId, string newName) {

    if (playerId <= 0) {
        return { -1, 400, "El ID del jugador no es válido." };
    }

    if (!isValidName(newName)) {
        return { -1, 400, "El nombre del jugador no puede estar vacío." };
    }

    return dbResponseFactory(connection_.updatePlayer(playerId, newName));
}

BackendResponse LogPlayer::eliminar(int playerId) {

    if (playerId <= 0) {
        return { -1, 400, "El ID del jugador no es válido." };
    }

    return dbResponseFactory(connection_.deletePlayer(playerId));
}
