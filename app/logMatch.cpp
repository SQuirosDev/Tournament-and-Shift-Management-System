#include <string>
#include <vector>
#include "logMatch.h"
#include "LogHistoric.h"
#include "factories.h"
#include "validations.h"

LogMatch::LogMatch(Connection& dbConnection) : connection_(dbConnection) {
    logHistoric = nullptr;
}

void LogMatch::setLogHistoric(LogHistoric* historic) {
    logHistoric = historic;
}

BackendResponse LogMatch::insert(int tournamentId, int teamAId, int teamBId) {

    if (tournamentId <= 0 || teamAId <= 0 || teamBId <= 0 || teamAId == teamBId) {
        return { -1, CODE_MATCH_INVALID_DATA, "Datos invalidos para crear el partido." };
    }

    BackendResponse response = dbResponseFactory(connection_.insertMatch(tournamentId, teamAId, teamBId, "Grupos", 0, 0));

    if (response.code >= 4000 && response.code < 5000) {
        return response;
    }

    string newData = "{ \"id\": " + to_string(response.id) + ", \"tournamentId\": " + to_string(tournamentId) + ", \"teamAId\": " + to_string(teamAId) + ", \"teamBId\": " + to_string(teamBId) + ", \"phase\": \"Grupos\", \"round\": 0, \"status\": \"Pendiente\", \"winnerId\": 0, \"result\": \"\", \"queuePosition\": 0, \"playedAt\": \"\" }";

    logHistoric->insert(historicFactory("Insert", "Match", response.id, "{}", newData));

    return response;
}

BackendQueryResponse<Match> LogMatch::listByTournament(int tournamentId) {

    if (tournamentId <= 0) {
        return { {}, CODE_MATCH_NOT_FOUND, "El ID del partido no es válido." };
    }

    BackendQueryResponse<Match> response =  dbQueryResponseFactory<Match>(connection_.listMatchesByTournament(tournamentId));
    return response;
}

BackendQueryResponse<Match> LogMatch::listByPhase(int id, string phase) {

    if (id <= 0) {
        return { {}, CODE_MATCH_NOT_FOUND, "El ID del partido no es válido." };
    }

    if (!isValidName(phase)) {
        return { {}, CODE_TEAM_INVALID_DATA, "El nombre de la fase no puede estar vacío." };
    }

    BackendQueryResponse<Match> response = dbQueryResponseFactory<Match>(connection_.listMatchesByPhase(id, phase));
    return response;
}

BackendResponse LogMatch::update(int id, string phase, int round, string status, int winnerId, string result) {

    if (id <= 0 || winnerId < 0) {
        return { -1, CODE_MATCH_INVALID_DATA, "Datos invalidos para actualizar marcador." };
    }

    if (round < 0 && round >= 3) {
        return { -1, CODE_MATCH_INVALID_DATA, "Datos invalidos para actualizar marcador." };
    }

    BackendQueryResponse<Match> queryResponse = dbQueryResponseFactory<Match>(connection_.obtainMatchById(id));

    if (queryResponse.data.empty()) {
        return { -1, CODE_MATCH_NOT_FOUND, "Partido no encontrado." };
    }

    Match match = queryResponse.data[0];

    string previousData = "{ \"id\": " + to_string(match.id) + ", \"tournamentId\": " + to_string(match.tournamentId) + ", \"teamAId\": " + to_string(match.teamAId) + ", \"teamBId\": " + to_string(match.teamBId) + ", \"phase\": \"" + match.phase + "\", \"round\": " + to_string(match.round) + ", \"status\": \"" + match.status + "\", \"winnerId\": " + to_string(match.winnerId) + ", \"result\": \"" + match.result + "\", \"queuePosition\": " + to_string(match.queuePosition) + " }";
    string newData = "{ \"id\": " + to_string(match.id) + ", \"tournamentId\": " + to_string(match.tournamentId) + ", \"teamAId\": " + to_string(match.teamAId) + ", \"teamBId\": " + to_string(match.teamBId) + ", \"phase\": \"" + phase + "\", \"round\": " + to_string(round) + ", \"status\": \"" + status + "\", \"winnerId\": " + to_string(winnerId) + ", \"result\": \"" + result + "\", \"queuePosition\": " + to_string(match.queuePosition) + " }";

    BackendResponse response = dbResponseFactory(connection_.updateMatch(id, phase, round, status, winnerId, result));

    if (response.code >= 4000 && response.code < 5000) {
        return response;
    }

    logHistoric->insert(historicFactory("Update", "Match", response.id, previousData, newData));

    return response;
}

BackendResponse LogMatch::eliminar(int id) {

    if (id <= 0) {
        return { -1, CODE_MATCH_NOT_FOUND, "El ID del partido no es válido." };
    }

    BackendQueryResponse<Match> queryResponse = dbQueryResponseFactory<Match>(connection_.obtainMatchById(id));

    if (queryResponse.data.empty()) {
        return { -1, CODE_MATCH_NOT_FOUND, "Partido no encontrado." };
    }

    Match match = queryResponse.data[0];

    string previousData = "{ \"id\": " + to_string(match.id) + ", \"tournamentId\": " + to_string(match.tournamentId) + ", \"teamAId\": " + to_string(match.teamAId) + ", \"teamBId\": " + to_string(match.teamBId) + ", \"phase\": \"" + match.phase + "\", \"round\": " + to_string(match.round) + ", \"status\": \"" + match.status + "\", \"winnerId\": " + to_string(match.winnerId) + ", \"result\": \"" + match.result + "\", \"queuePosition\": " + to_string(match.queuePosition) + " }";

    BackendResponse response = dbResponseFactory(connection_.deleteMatch(id));

    if (response.code >= 4000 && response.code < 5000) {
        return response;
    }

    logHistoric->insert(historicFactory("Delete", "Match", response.id, previousData, "{}"));

    return response;
}
