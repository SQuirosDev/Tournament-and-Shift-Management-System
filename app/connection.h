#pragma once

#include <string>
#include <vector>
#include "sqlite3.h"
#include "codes.h"
#include "tournaments.h"
#include "teams.h"
#include "players.h"
#include "matches.h"
#include "petitions.h"
#include "historics.h"

using namespace std;

struct DbResponse {
    int    id = -1;
    int    code = 0;
    string message;
};

class Connection {
public:

    Connection();
    ~Connection();

    DbResponse open(string dbPath);
    void close();
    bool isOpen();

    // ============================================================
    //  TB_TOURNAMENT
    // ============================================================
    DbResponse insertTournament(string name);
    DbResponse listTournaments(vector<Tournament>& outputList);
    DbResponse obtainTournamentById(int id, Tournament& outputRow);
    DbResponse updateTournamentPhase(int id, string phase);
    DbResponse updateTournamentName(int id, string newName);

    // ============================================================
    //  TB_TEAM
    // ============================================================
    DbResponse insertTeam(string name, int tournamentId);
    DbResponse listTeamsByTournament(int tournamentId, vector<Team>& outputList);
    DbResponse obtainTeamById(int id, Team& outputRow);
    DbResponse updateTeam(int id, string newName);
    DbResponse updateTeamStats(int id, int points, int wins, int draws, int losses);
    DbResponse deleteTeam(int id);

    // ============================================================
    //  TB_PLAYER
    // ============================================================
    DbResponse insertPlayer(string name, int teamId);
    DbResponse listPlayersByTeam(int teamId, vector<Player>& outputList);
    DbResponse obtainPlayerById(int id, Player& outputRow);
    DbResponse updatePlayer(int id, string name);
    DbResponse deletePlayer(int id);

    // ============================================================
    //  TB_MATCH
    // ============================================================
    DbResponse insertMatch(int tournamentId, int teamAId, int teamBId, string phase, int round, int queuePosition);
    DbResponse listMatchesByTournament(int tournamentId, vector<Match>& outputList);
    DbResponse listMatchesByPhase(int tournamentId, string phase, vector<Match>& outputList);
    DbResponse obtainNextMatch(int tournamentId, Match& outputRow);
    DbResponse updateMatchResult(int id, string result, int winnerId, int round);
    DbResponse deleteMatch(int id);

    // ============================================================
    //  TB_PETITION
    // ============================================================
    DbResponse insertPetition(string requesterName, string type, string description);
    DbResponse listPendingPetitions(vector<Petition>& outputList);
    DbResponse obtainNextPetition(Petition& outputRow);
    DbResponse updatePetitionStatus(int id, string status);

    // ============================================================
    //  TB_HISTORIC (Pila / Deshacer)
    // ============================================================
    DbResponse insertHistoric(int tournamentId, string actionType, string previousData, string newData, int stackPosition);
    DbResponse listHistoricByTournament(int tournamentId, vector<Historic>& outputList);
    DbResponse obtainLastHistoric(int tournamentId, Historic& outputRow);
    DbResponse deleteLastHistoric(int tournamentId);
    DbResponse deleteHistoricById(int id);

private:

    sqlite3* db_;
    bool opened_;

    DbResponse initTables();
    DbResponse sqliteError(int code, string context);
};