#include "logGame.h"
#include <vector>
#include <cstdlib>
#include "codes.h"
#include "factories.h"
#include "Matches.h"
#include "Teams.h"
#include "queueMatch.h"

using namespace std;

LogGame::LogGame(Connection& dbConnection) : connection_(dbConnection) {}

// =========================
// GRUPOS
// =========================

BackendResponse LogGame::generateGroupMatches(int tournamentId) {

    BackendQueryResponse<Team> teamResponse = dbQueryResponseFactory(connection_.listTeamsByTournament(tournamentId));

    if (teamResponse.data.size() < 2) {
        return { -1, CODE_GAME_INVALID_SIZE_TEAM, "No hay suficientes equipos." };
    }

    int queuePosition = 1;

    for (int i = 0; i < teamResponse.data.size(); i++) {
        for (int j = i + 1; j < teamResponse.data.size(); j++) {
            connection_.insertMatch(tournamentId, teamResponse.data[i].id, teamResponse.data[j].id, "Grupos", 0, queuePosition);
            queuePosition++;
        }
    }

    return { 0, CODE_GAME_CREATED, "Partidos de grupos generados correctamente." };
}

BackendResponse LogGame::playGroupMatches(int tournamentId) {

    BackendQueryResponse<Match> matchResponse = dbQueryResponseFactory(connection_.listMatchesByPhase(tournamentId, "Grupos"));

    if (matchResponse.data.empty()) {
        return { -1, CODE_GAME_INVALID_MATCHES, "No hay partidos de grupos." };
    }

    QueueMatch matchQueue;

    // Encolar en orden (ya viene ordenado por queue_position)
    for (int i = 0; i < matchResponse.data.size(); i++) {
        if (matchResponse.data[i].status != "Finalizado") {
            matchQueue.enqueue(matchResponse.data[i]);
        }
    }

    while (!matchQueue.isEmpty()) {

        Match match = matchQueue.front();
        matchQueue.dequeue();

        int scoreA = rand() % 5;
        int scoreB = rand() % 5;

        int winnerId = 0;
        int teamA = match.teamAId;
        int teamB = match.teamBId;

        if (scoreA > scoreB) {
            winnerId = teamA;
            connection_.updateTeamStats(teamA, 3, 1, 0, 0);
            connection_.updateTeamStats(teamB, 0, 0, 0, 1);
        }
        else if (scoreB > scoreA) {
            winnerId = teamB;
            connection_.updateTeamStats(teamB, 3, 1, 0, 0);
            connection_.updateTeamStats(teamA, 0, 0, 0, 1);
        }
        else {
            winnerId = 0;
            connection_.updateTeamStats(teamA, 1, 0, 1, 0);
            connection_.updateTeamStats(teamB, 1, 0, 1, 0);
        }

        string result = to_string(scoreA) + "-" + to_string(scoreB);

        connection_.updateMatch(match.id, "Grupos", 0, "Finalizado", winnerId, result);
    }

    return { 0, CODE_GAME_GROUP_PHASE_ENDED, "Fase de grupos finalizada." };
}

// =========================
// SEMIFINALES
// =========================

BackendResponse LogGame::generateSemiMatches(int tournamentId) {

    BackendQueryResponse<Team> teamResponse = dbQueryResponseFactory(connection_.listTeamsByTournament(tournamentId));

    if (teamResponse.data.size() < 4) {
        return { -1, CODE_GAME_INVALID_SIZE_TEAM, "No hay suficientes equipos para semifinal." };
    }

    vector<Team> topTeams;
    vector<Team> tempTeams = teamResponse.data;

    while (topTeams.size() < 4) {

        int bestIndex = 0;

        for (int i = 1; i < tempTeams.size(); i++) {
            if (tempTeams[i].points > tempTeams[bestIndex].points) {
                bestIndex = i;
            }
        }

        topTeams.push_back(tempTeams[bestIndex]);
        tempTeams.erase(tempTeams.begin() + bestIndex);
    }

    connection_.insertMatch(tournamentId, topTeams[0].id, topTeams[3].id, "Eliminacion", 1, 1);
    connection_.insertMatch(tournamentId, topTeams[1].id, topTeams[2].id, "Eliminacion", 1, 2);
    
    return { 0, CODE_GAME_CREATED, "Semifinales generadas." };
}

BackendResponse LogGame::playSemiMatches(int tournamentId) {

    BackendQueryResponse<Match> matchResponse = dbQueryResponseFactory(connection_.listMatchesByPhase(tournamentId, "Eliminacion"));

    if (matchResponse.data.empty()) {
        return { -1, CODE_GAME_INVALID_MATCHES, "No hay partidos de semifinal." };
    }

    QueueMatch matchQueue;

    for (int i = 0; i < matchResponse.data.size(); i++) {
        if (matchResponse.data[i].status != "Finalizado" && matchResponse.data[i].round == 1) {
            matchQueue.enqueue(matchResponse.data[i]);
        }
    }

    while (!matchQueue.isEmpty()) {

        Match match = matchQueue.front();
        matchQueue.dequeue();

        int scoreA = rand() % 5;
        int scoreB = rand() % 5;

        while (scoreA == scoreB) {
            scoreA = rand() % 5;
            scoreB = rand() % 5;
        }

        int winnerId = 0;

        if (scoreA > scoreB) {
            winnerId = match.teamAId;
        }
        else {
            winnerId = match.teamBId;
        }

        string result = to_string(scoreA) + "-" + to_string(scoreB);

        connection_.updateMatch(match.id, "Eliminacion", 1, "Finalizado", winnerId, result);
    }

    return { 0, CODE_GAME_ELIMINATION_PHASE_ENDED, "Semifinales finalizadas." };
}

// =========================
// FINAL
// =========================

BackendResponse LogGame::generateFinalMatch(int tournamentId) {

    BackendQueryResponse<Match> matchResponse = dbQueryResponseFactory(connection_.listMatchesByPhase(tournamentId, "Eliminacion"));

    vector<int> winners;

    for (int i = 0; i < matchResponse.data.size(); i++) {
        if (matchResponse.data[i].round == 1 && matchResponse.data[i].status == "Finalizado") {
            winners.push_back(matchResponse.data[i].winnerId);
        }
    }

    if (winners.size() != 2) {
        return { -1, CODE_GAME_INVALID_SIZE_TEAM, "No hay suficientes ganadores para la final." };
    }

    connection_.insertMatch(tournamentId, winners[0], winners[1], "Eliminacion", 2, 1);

    return { 0, CODE_GAME_CREATED, "Final generada." };
}

BackendResponse LogGame::playFinalMatch(int tournamentId) {

    BackendQueryResponse<Match> matchResponse = dbQueryResponseFactory(connection_.listMatchesByPhase(tournamentId, "Eliminacion"));

    for (int i = 0; i < matchResponse.data.size(); i++) {

        Match match = matchResponse.data[i];

        if (match.round != 2 || match.status == "Finalizado") {
            continue;
        }

        int scoreA = rand() % 5;
        int scoreB = rand() % 5;

        while (scoreA == scoreB) {
            scoreA = rand() % 5;
            scoreB = rand() % 5;
        }

        int winnerId = 0;

        if (scoreA > scoreB) {
            winnerId = match.teamAId;
        }
        else {
            winnerId = match.teamBId;
        }

        string result = to_string(scoreA) + "-" + to_string(scoreB);

        connection_.updateMatch(match.id, "Eliminacion", 2, "Finalizado", winnerId, result);
    }

    return { 0, CODE_GAME_ELIMINATION_PHASE_ENDED, "Final jugada correctamente." };
}
