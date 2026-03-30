#pragma once

#include <string>
#include <vector>
#include "sqlite3.h"
#include "codes.h"
#include "dbResponse.h"
#include "dbQueryResponse.h"
#include "tournaments.h"
#include "teams.h"
#include "players.h"
#include "matches.h"
#include "petitions.h"
#include "historics.h"

using namespace std;

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
    DBQueryResponse<Tournament> listTournaments();
    DBQueryResponse<Tournament> obtainTournamentById(int id);
    DbResponse updateTournamentPhase(int id, string phase);
    DbResponse updateTournamentName(int id, string newName);

    // ============================================================
    //  TB_TEAM
    // ============================================================
    DbResponse insertTeam(string name, int tournamentId);
    DBQueryResponse<Team> listTeamsByTournament(int tournamentId);
    DBQueryResponse<Team> obtainTeamById(int id);
    DbResponse updateTeam(int id, string newName);
    DbResponse updateTeamStats(int id, int points, int wins, int draws, int losses);
    DbResponse deleteTeam(int id);

    // ============================================================
    //  TB_PLAYER
    // ============================================================
    DbResponse insertPlayer(string name, int teamId);
    DBQueryResponse<Player> listPlayersByTeam(int teamId);
    DBQueryResponse<Player> obtainPlayerById(int id);
    DbResponse updatePlayer(int id, string name);
    DbResponse deletePlayer(int id);

    // ============================================================
    //  TB_MATCH
    // ============================================================
    DbResponse insertMatch(int tournamentId, int teamAId, int teamBId, string phase, int round, int queuePosition);
    DBQueryResponse<Match> listMatchesByTournament(int tournamentId);
    DBQueryResponse<Match> listMatchesByPhase(int tournamentId, string phase);
    DBQueryResponse<Match> obtainNextMatch(int tournamentId);
    DbResponse updateMatchResult(int id, string result, int winnerId, int round);
    DbResponse deleteMatch(int id);

    // ============================================================
    //  TB_PETITION
    // ============================================================
    DbResponse insertPetition(string requesterName, string type, string description);
    DBQueryResponse<Petition> listPendingPetitions();
    DBQueryResponse<Petition> obtainNextPetition();
    DbResponse updatePetitionStatus(int id, string response, string status);
    DBQueryResponse<Petition> obtainPetitionById(int id);
    DbResponse deletePetition(int id);

    // ============================================================
    //  TB_HISTORIC (Pila / Deshacer)
    // ============================================================
    DbResponse insertHistoric(string actionType, string entityType, int recordId, string previousData, string newData, int stackPosition);
    DBQueryResponse<Historic> listHistoric();
    DbResponse deleteLastHistoric();

private:

    sqlite3* db_;
    bool opened_;

    DbResponse initTables();
    DbResponse sqliteError(int code, string context);
};
