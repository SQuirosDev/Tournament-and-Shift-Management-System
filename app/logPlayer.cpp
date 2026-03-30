#include "logPlayer.h"
#include "LogHistoric.h"
#include "validations.h"
#include "factories.h"

LogPlayer::LogPlayer(Connection& dbConnection) : connection_(dbConnection) {
    logHistoric = nullptr;
}

void LogPlayer::setLogHistoric(LogHistoric* historic) {
    logHistoric = historic;
}

BackendResponse LogPlayer::insert(int teamId, string playerName) {

    if (teamId <= 0) {
        return { -1, 400, "El ID del equipo no es válido." };
    }

    if (!isValidName(playerName)) {
        return { 0, 400, "El nombre del jugador no puede estar vacío." };
    }

    BackendResponse response = dbResponseFactory(connection_.insertPlayer(playerName, teamId));

    if (response.id <= 0) {
        return response;
    }

    string newData = "{ \"id\": " + to_string(response.id) + ", \"teamId\": " + to_string(teamId) + ", \"name\": \"" + playerName + "\" }";

    logHistoric->insert(historicFactory("Insert", "Player", response.id, "{}", newData));

    return response;
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

    BackendQueryResponse<Player> queryResponse = dbQueryResponseFactory<Player>(connection_.obtainPlayerById(playerId));

    if (queryResponse.data.empty()) {
        return { -1, 404, "Jugador no encontrado." };
    }

    Player player = queryResponse.data[0];

    string previousData = "{ \"id\": " + to_string(player.id) + ", \"teamId\": " + to_string(player.teamId) + ", \"name\": \"" + player.name + "\" }";
    string newData = "{ \"id\": " + to_string(player.id) + ", \"teamId\": " + to_string(player.teamId) + ", \"name\": \"" + newName + "\" }";

    BackendResponse response = dbResponseFactory(connection_.updatePlayer(playerId, newName));

    if (response.id <= 0) {
        return response;
    }

    logHistoric->insert(historicFactory("Update", "Player", response.id, previousData, newData));

    return response;
}

BackendResponse LogPlayer::eliminar(int playerId) {

    if (playerId <= 0) {
        return { -1, 400, "El ID del jugador no es válido." };
    }

    BackendQueryResponse<Player> queryResponse = dbQueryResponseFactory<Player>(connection_.obtainPlayerById(playerId));

    if (queryResponse.data.empty()) {
        return { -1, 404, "Jugador no encontrado." };
    }

    Player player = queryResponse.data[0];

    string previousData = "{ \"id\": " + to_string(player.id) + ", \"teamId\": " + to_string(player.teamId) + ", \"name\": \"" + player.name + "\" }";

    BackendResponse response = dbResponseFactory(connection_.deletePlayer(playerId));

    if (response.id <= 0) {
        return response;
    }

    logHistoric->insert(historicFactory("Delete", "Player", response.id, previousData, "{}"));

    return response;
}
