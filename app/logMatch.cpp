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
        return { {}, CODE_MATCH_NOT_FOUND, "El ID del torneo no es válido." };
    }

    BackendQueryResponse<Match> response =  dbQueryResponseFactory<Match>(connection_.listMatchesByTournament(tournamentId));
    return response;
}

BackendQueryResponse<Match> LogMatch::listByPhase(int id, string phase) {

    if (id <= 0) {
        return { {}, CODE_MATCH_NOT_FOUND, "El ID del torneo no es válido." };
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

    if (round < 0 || round >= 3) {
        return { -1, CODE_MATCH_INVALID_DATA, "Datos invalidos para actualizar marcador." };
    }

    BackendQueryResponse<Match> queryResponse = dbQueryResponseFactory<Match>(connection_.obtainMatchById(id));

    if (queryResponse.data.empty()) {
        return { -1, CODE_MATCH_NOT_FOUND, "Partido no encontrado." };
    }

    Match match = queryResponse.data[0];

    BackendResponse response = dbResponseFactory(connection_.updateMatch(id, phase, round, status, winnerId, result));

    if (response.code >= 4000 && response.code < 5000) {
        return response;
    }

    if (match.status == "Finalizado") {

        int teamAId = match.teamAId;
        int teamBId = match.teamBId;

        int pointsA = 0, winsA = 0, drawsA = 0, lossesA = 0;
        int pointsB = 0, winsB = 0, drawsB = 0, lossesB = 0;

        // Empate
        if (winnerId == 0) {
            pointsA = 1;
            drawsA = 1;

            pointsB = 1;
            drawsB = 1;
        }
        // Gana A
        else if (winnerId == teamAId) {
            pointsA = 3;
            winsA = 1;

            lossesB = 1;
        }
        // Gana B
        else if (winnerId == teamBId) {
            pointsB = 3;
            winsB = 1;

            lossesA = 1;
        }
        else {
            return { -1, CODE_MATCH_INVALID_DATA, "WinnerId no corresponde a los equipos" };
        }

        // Actualizar Team A
        BackendQueryResponse<Team> queryResponseA = dbQueryResponseFactory<Team>(connection_.obtainTeamById(teamAId));

        if (queryResponseA.data.empty()) {
            return { -1, CODE_TEAM_NOT_FOUND, "Equipo no encontrado." };
        }

        Team teamA = queryResponseA.data[0];

        BackendResponse teamAResponseA = dbResponseFactory(connection_.updateTeamStats(teamAId, teamA.points + pointsA, teamA.wins + winsA, teamA.draws + drawsA, teamA.losses + lossesA));

        if (teamAResponseA.code >= 4000 && teamAResponseA.code < 5000) {
            return teamAResponseA;
        }

        // Actualizar Team B
        BackendQueryResponse<Team> queryResponseB = dbQueryResponseFactory<Team>(connection_.obtainTeamById(teamBId));

        if (queryResponseB.data.empty()) {
            return { -1, CODE_TEAM_NOT_FOUND, "Equipo no encontrado." };
        }

        Team teamB = queryResponseB.data[0];

        BackendResponse teamBResponseB = dbResponseFactory(connection_.updateTeamStats(teamBId, teamB.points + pointsB, teamB.wins + winsB, teamB.draws + drawsB, teamB.losses + lossesB));

        if (teamBResponseB.code >= 4000 && teamBResponseB.code < 5000) {
            return teamBResponseB;
        }
    }

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
