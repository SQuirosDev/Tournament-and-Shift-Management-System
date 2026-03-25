#pragma once

#include <string>

using namespace std;

struct Match {
    int    id = 0;
    int    tournamentId = 0;
    int    teamAId = 0;
    int    teamBId = 0;
    string phase;         // "Grupos" | "Eliminacion"
    int    round = 1;
    string status;        // "Pendiente" | "Finalizado"
    int    winnerId = 0;
    string result;        // "Gana A" | "Gana B" | "Empate"
    int    queuePosition = 0;
    string playedAt;
};