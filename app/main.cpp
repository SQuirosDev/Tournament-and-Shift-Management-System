//#include <iostream>
#include <QDebug>
#include <QtWidgets/QApplication>
#include "app.h"
#include "connection.h"

using namespace std;

void printResponse(string functionName, DbResponse response);
void testTournament(Connection& connection);
void testTeam(Connection& connection, int tournamentId);
void testPlayer(Connection& connection, int teamId);
void testMatch(Connection& connection, int tournamentId, int teamAId, int teamBId);
void testPetition(Connection& connection);
void testHistoric(Connection& connection, int tournamentId);

int main(int argc, char *argv[])
{
    //QApplication a(argc, argv);
    //app window;
    //window.show();
    //return a.exec();

    Connection connection;

    // Abrir la base de datos (se crea si no existe)
    DbResponse openResponse = connection.open("test_tournament.db");
    printResponse("open", openResponse);

    if (!connection.isOpen()) {
        qDebug() << "No se pudo abrir la base de datos.";
        return 1;
    }

    // -- TOURNAMENT --
    testTournament(connection);

    // Insertar un torneo base para las pruebas siguientes
    DbResponse baseTournamentResponse = connection.insertTournament("Torneo Base");
    int baseTournamentId = baseTournamentResponse.id;

    // -- TEAM --
    testTeam(connection, baseTournamentId);

    // Insertar equipos base para pruebas de Match y Player
    DbResponse teamAResponse = connection.insertTeam("Team A", baseTournamentId);
    DbResponse teamBResponse = connection.insertTeam("Team B", baseTournamentId);
    int baseTeamAId = teamAResponse.id;
    int baseTeamBId = teamBResponse.id;

    // -- PLAYER --
    testPlayer(connection, baseTeamAId);

    // -- MATCH --
    testMatch(connection, baseTournamentId, baseTeamAId, baseTeamBId);

    // -- PETITION --
    testPetition(connection);

    // -- HISTORIC --
    testHistoric(connection, baseTournamentId);

    connection.close();
    qDebug() << "\n========== Pruebas finalizadas ==========";

    return 0;
}



// ------------------------------------------------------------
//  Helper para imprimir el resultado de cada operacion
// ------------------------------------------------------------
void printResponse(string functionName, DbResponse response) {
    qDebug() << "\n---" << QString::fromStdString(functionName) << "---";
    qDebug() << "ID      :" << response.id;
    qDebug() << "Codigo  :" << response.code;
    qDebug() << "Mensaje :" << QString::fromStdString(response.message);
}

// ------------------------------------------------------------
//  Pruebas TB_TOURNAMENT
// ------------------------------------------------------------
void testTournament(Connection& connection) {
    qDebug() << "\n========== TB_TOURNAMENT ==========";

    // INSERT
    DbResponse insertResponse = connection.insertTournament("Copa 2026");
    printResponse("insertTournament", insertResponse);

    // INSERT duplicado (debe fallar)
    DbResponse insertDuplicateResponse = connection.insertTournament("Copa 2026");
    printResponse("insertTournament (duplicado)", insertDuplicateResponse);

    // INSERT vacio (debe fallar)
    DbResponse insertEmptyResponse = connection.insertTournament("");
    printResponse("insertTournament (vacio)", insertEmptyResponse);

    // LIST
    vector<Tournament> tournamentList;
    DbResponse listResponse = connection.listTournaments(tournamentList);
    printResponse("listTournaments", listResponse);
    for (Tournament tournamentRow : tournamentList)
        qDebug() << "  -> ID:" << tournamentRow.id << "| Nombre:" << QString::fromStdString(tournamentRow.name) << "| Fase:" << QString::fromStdString(tournamentRow.phase);


    // OBTAIN BY ID
    Tournament tournamentRow;
    DbResponse obtainResponse = connection.obtainTournamentById(insertResponse.id, tournamentRow);
    printResponse("obtainTournamentById", obtainResponse);
    if (obtainResponse.code == CODE_TOURNAMENT_LISTED)
        qDebug() << "  -> ID:" << tournamentRow.id << "| Nombre:" << QString::fromStdString(tournamentRow.name) << "| Fase:" << QString::fromStdString(tournamentRow.phase);

    // UPDATE PHASE
    DbResponse updatePhaseResponse = connection.updateTournamentPhase(insertResponse.id, "Grupos");
    printResponse("updateTournamentPhase", updatePhaseResponse);

    // UPDATE PHASE invalida (debe fallar)
    DbResponse updatePhaseInvalidResponse = connection.updateTournamentPhase(insertResponse.id, "Semifinal");
    printResponse("updateTournamentPhase (invalida)", updatePhaseInvalidResponse);

    // UPDATE NAME
    DbResponse updateNameResponse = connection.updateTournamentName(insertResponse.id, "Copa Mundial 2026");
    printResponse("updateTournamentName", updateNameResponse);

    // UPDATE ID inexistente (debe fallar)
    DbResponse updateNotFoundResponse = connection.updateTournamentName(9999, "Nadie");
    printResponse("updateTournamentName (no existe)", updateNotFoundResponse);
}


// ------------------------------------------------------------
//  Pruebas TB_TEAM
// ------------------------------------------------------------
void testTeam(Connection& connection, int tournamentId) {
    qDebug() << "\n========== TB_TEAM ==========";

    // INSERT
    DbResponse insertResponse = connection.insertTeam("Equipo Alpha", tournamentId);
    printResponse("insertTeam", insertResponse);

    connection.insertTeam("Equipo Beta", tournamentId);
    connection.insertTeam("Equipo Gamma", tournamentId);

    // INSERT duplicado (debe fallar)
    DbResponse insertDuplicateResponse = connection.insertTeam("Equipo Alpha", tournamentId);
    printResponse("insertTeam (duplicado)", insertDuplicateResponse);

    // INSERT torneo inexistente (debe fallar)
    DbResponse insertNoTournamentResponse = connection.insertTeam("Equipo X", 9999);
    printResponse("insertTeam (torneo no existe)", insertNoTournamentResponse);

    // LIST
    vector<Team> teamList;
    DbResponse listResponse = connection.listTeamsByTournament(tournamentId, teamList);
    printResponse("listTeamsByTournament", listResponse);
    for (Team teamRow : teamList)
        qDebug() << "  -> ID:" << teamRow.id << "| Nombre:" << QString::fromStdString(teamRow.name) << "| Puntos:" << teamRow.points;

    // OBTAIN BY ID
    Team teamRow;
    DbResponse obtainResponse = connection.obtainTeamById(insertResponse.id, teamRow);
    printResponse("obtainTeamById", obtainResponse);
    if (obtainResponse.code == CODE_TEAM_LISTED)
        qDebug() << "  -> ID:" << teamRow.id << "| Nombre:" << QString::fromStdString(teamRow.name);

    // UPDATE NAME
    DbResponse updateResponse = connection.updateTeam(insertResponse.id, "Equipo Alpha Renovado");
    printResponse("updateTeam", updateResponse);

    // UPDATE STATS
    DbResponse updateStatsResponse = connection.updateTeamStats(insertResponse.id, 9, 3, 0, 0);
    printResponse("updateTeamStats", updateStatsResponse);

    // DELETE
    DbResponse deleteResponse = connection.deleteTeam(insertResponse.id);
    printResponse("deleteTeam", deleteResponse);

    // DELETE inexistente (debe fallar)
    DbResponse deleteNotFoundResponse = connection.deleteTeam(9999);
    printResponse("deleteTeam (no existe)", deleteNotFoundResponse);
}


// ------------------------------------------------------------
//  Pruebas TB_PLAYER
// ------------------------------------------------------------
void testPlayer(Connection& connection, int teamId) {
    qDebug() << "\n========== TB_PLAYER ==========";

    // INSERT
    DbResponse insertResponse = connection.insertPlayer("Juan Perez", teamId);
    printResponse("insertPlayer", insertResponse);

    connection.insertPlayer("Maria Lopez", teamId);
    connection.insertPlayer("Carlos Mora", teamId);

    // INSERT duplicado (debe fallar)
    DbResponse insertDuplicateResponse = connection.insertPlayer("Juan Perez", teamId);
    printResponse("insertPlayer (duplicado)", insertDuplicateResponse);

    // INSERT equipo inexistente (debe fallar)
    DbResponse insertNoTeamResponse = connection.insertPlayer("Nadie", 9999);
    printResponse("insertPlayer (equipo no existe)", insertNoTeamResponse);

    // LIST
    vector<Player> playerList;
    DbResponse listResponse = connection.listPlayersByTeam(teamId, playerList);
    printResponse("listPlayersByTeam", listResponse);
    for (Player playerRow : playerList)
        qDebug() << "  -> ID:" << playerRow.id << "| Nombre:" << QString::fromStdString(playerRow.name) << "| Equipo ID:" << playerRow.teamId;

    // OBTAIN BY ID
    Player playerRow;
    DbResponse obtainResponse = connection.obtainPlayerById(insertResponse.id, playerRow);
    printResponse("obtainPlayerById", obtainResponse);
    if (obtainResponse.code == CODE_PLAYER_LISTED)
        qDebug() << "  -> ID:" << playerRow.id << "| Nombre:" << QString::fromStdString(playerRow.name);

    // UPDATE
    DbResponse updateResponse = connection.updatePlayer(insertResponse.id, "Juan Perez Actualizado");
    printResponse("updatePlayer", updateResponse);

    // DELETE
    DbResponse deleteResponse = connection.deletePlayer(insertResponse.id);
    printResponse("deletePlayer", deleteResponse);

    // DELETE inexistente (debe fallar)
    DbResponse deleteNotFoundResponse = connection.deletePlayer(9999);
    printResponse("deletePlayer (no existe)", deleteNotFoundResponse);
}


// ------------------------------------------------------------
//  Pruebas TB_MATCH
// ------------------------------------------------------------
void testMatch(Connection& connection, int tournamentId, int teamAId, int teamBId) {
    qDebug() << "\n========== TB_MATCH ==========";

    // INSERT
    DbResponse insertResponse = connection.insertMatch(tournamentId, teamAId, teamBId, "Grupos", 1, 1);
    printResponse("insertMatch", insertResponse);

    // INSERT mismo equipo contra si mismo (debe fallar)
    DbResponse insertSameTeamResponse = connection.insertMatch(tournamentId, teamAId, teamAId, "Grupos", 1, 2);
    printResponse("insertMatch (mismo equipo)", insertSameTeamResponse);

    // INSERT fase invalida (debe fallar)
    DbResponse insertInvalidPhaseResponse = connection.insertMatch(tournamentId, teamAId, teamBId, "Semifinal", 1, 3);
    printResponse("insertMatch (fase invalida)", insertInvalidPhaseResponse);

    // LIST BY TOURNAMENT
    vector<Match> matchList;
    DbResponse listResponse = connection.listMatchesByTournament(tournamentId, matchList);
    printResponse("listMatchesByTournament", listResponse);
    for (Match matchRow : matchList)
        qDebug() << "  -> ID:" << matchRow.id << "| Equipo A:" << matchRow.teamAId << "vs Equipo B:" << matchRow.teamBId << "| Estado:" << QString::fromStdString(matchRow.status);

    // LIST BY PHASE
    vector<Match> matchByPhaseList;
    DbResponse listByPhaseResponse = connection.listMatchesByPhase(tournamentId, "Grupos", matchByPhaseList);
    printResponse("listMatchesByPhase", listByPhaseResponse);

    // OBTAIN NEXT
    Match nextMatchRow;
    DbResponse obtainNextResponse = connection.obtainNextMatch(tournamentId, nextMatchRow);
    printResponse("obtainNextMatch", obtainNextResponse);
    if (obtainNextResponse.code == CODE_MATCH_LISTED)
        qDebug() << "  -> Siguiente partido ID:" << nextMatchRow.id << "| Estado:" << QString::fromStdString(nextMatchRow.status);

    // UPDATE RESULT
    DbResponse updateResponse = connection.updateMatchResult(insertResponse.id, "Gana A", teamAId, 1);
    printResponse("updateMatchResult", updateResponse);

    // UPDATE partido ya finalizado (debe fallar)
    DbResponse updateAlreadyDoneResponse = connection.updateMatchResult(insertResponse.id, "Gana B", teamBId, 1);
    printResponse("updateMatchResult (ya finalizado)", updateAlreadyDoneResponse);

    // DELETE
    DbResponse deleteResponse = connection.deleteMatch(insertResponse.id);
    printResponse("deleteMatch", deleteResponse);

    // DELETE inexistente (debe fallar)
    DbResponse deleteNotFoundResponse = connection.deleteMatch(9999);
    printResponse("deleteMatch (no existe)", deleteNotFoundResponse);
}


// ------------------------------------------------------------
//  Pruebas TB_PETITION
// ------------------------------------------------------------
void testPetition(Connection& connection) {
    qDebug() << "\n========== TB_PETITION ==========";

    // INSERT
    DbResponse insertResponse1 = connection.insertPetition("Ana Gomez", "Inscripcion", "Quiero inscribir a mi equipo");
    printResponse("insertPetition (1)", insertResponse1);

    DbResponse insertResponse2 = connection.insertPetition("Luis Torres", "Consulta", "Cuando inicia el torneo?");
    printResponse("insertPetition (2)", insertResponse2);

    // INSERT tipo invalido (debe fallar)
    DbResponse insertInvalidTypeResponse = connection.insertPetition("Pedro", "Queja", "No me gusta el horario");
    printResponse("insertPetition (tipo invalido)", insertInvalidTypeResponse);

    // INSERT nombre vacio (debe fallar)
    DbResponse insertEmptyNameResponse = connection.insertPetition("", "Consulta", "Sin nombre");
    printResponse("insertPetition (nombre vacio)", insertEmptyNameResponse);

    // LIST PENDING
    vector<Petition> petitionList;
    DbResponse listResponse = connection.listPendingPetitions(petitionList);
    printResponse("listPendingPetitions", listResponse);
    for (Petition petitionRow : petitionList)
        qDebug() << "  -> ID:" << petitionRow.id << "| Solicitante:" << QString::fromStdString(petitionRow.requesterName) << "| Tipo:" << QString::fromStdString(petitionRow.type) << "| Posicion:" << petitionRow.queuePosition;

    // OBTAIN NEXT
    Petition nextPetitionRow;
    DbResponse obtainNextResponse = connection.obtainNextPetition(nextPetitionRow);
    printResponse("obtainNextPetition", obtainNextResponse);
    if (obtainNextResponse.code == CODE_PETITION_LISTED)
        qDebug() << "  -> Siguiente:" << QString::fromStdString(nextPetitionRow.requesterName) << "| Tipo:" << QString::fromStdString(nextPetitionRow.type);

    // UPDATE STATUS -> Atendida
    DbResponse updateResponse = connection.updatePetitionStatus(insertResponse1.id, "Atendida");
    printResponse("updatePetitionStatus (Atendida)", updateResponse);

    // UPDATE STATUS -> ya atendida (debe fallar)
    DbResponse updateAlreadyDoneResponse = connection.updatePetitionStatus(insertResponse1.id, "Cancelada");
    printResponse("updatePetitionStatus (ya procesada)", updateAlreadyDoneResponse);

    // UPDATE STATUS -> Cancelada
    DbResponse cancelResponse = connection.updatePetitionStatus(insertResponse2.id, "Cancelada");
    printResponse("updatePetitionStatus (Cancelada)", cancelResponse);
}


// ------------------------------------------------------------
//  Pruebas TB_HISTORIC
// ------------------------------------------------------------
void testHistoric(Connection& connection, int tournamentId) {
    qDebug() << "\n========== TB_HISTORIC ==========";

    // INSERT
    DbResponse insertResponse1 = connection.insertHistoric(tournamentId, "Crear Torneo", "sin torneo", "torneo creado", 1);
    printResponse("insertHistoric (1)", insertResponse1);

    DbResponse insertResponse2 = connection.insertHistoric(tournamentId, "Registrar Jugador", "sin jugador", "jugador registrado", 2);
    printResponse("insertHistoric (2)", insertResponse2);

    DbResponse insertResponse3 = connection.insertHistoric(tournamentId, "Generar Equipos", "sin equipos", "equipos generados", 3);
    printResponse("insertHistoric (3)", insertResponse3);

    // INSERT tipo vacio (debe fallar)
    DbResponse insertEmptyTypeResponse = connection.insertHistoric(tournamentId, "", "dato previo", "dato nuevo", 4);
    printResponse("insertHistoric (tipo vacio)", insertEmptyTypeResponse);

    // INSERT previousData vacio (debe fallar)
    DbResponse insertEmptyDataResponse = connection.insertHistoric(tournamentId, "Crear Torneo", "", "dato nuevo", 4);
    printResponse("insertHistoric (previousData vacio)", insertEmptyDataResponse);

    // LIST
    vector<Historic> historicList;
    DbResponse listResponse = connection.listHistoricByTournament(tournamentId, historicList);
    printResponse("listHistoricByTournament", listResponse);
    for (Historic historicRow : historicList)
        qDebug() << "  -> ID:" << historicRow.id << "| Accion:" << QString::fromStdString(historicRow.actionType) << "| Stack:" << historicRow.stackPosition;

    // OBTAIN LAST (tope de la pila)
    Historic lastHistoricRow;
    DbResponse obtainLastResponse = connection.obtainLastHistoric(tournamentId, lastHistoricRow);
    printResponse("obtainLastHistoric", obtainLastResponse);
    if (obtainLastResponse.code == CODE_HISTORIC_LISTED)
        qDebug() << "  -> Ultima accion:" << QString::fromStdString(lastHistoricRow.actionType) << "| Stack:" << lastHistoricRow.stackPosition;

    // DELETE LAST (deshacer)
    DbResponse deleteLastResponse = connection.deleteLastHistoric(tournamentId);
    printResponse("deleteLastHistoric", deleteLastResponse);

    // DELETE BY ID
    DbResponse deleteByIdResponse = connection.deleteHistoricById(insertResponse1.id);
    printResponse("deleteHistoricById", deleteByIdResponse);

    // DELETE inexistente (debe fallar)
    DbResponse deleteNotFoundResponse = connection.deleteHistoricById(9999);
    printResponse("deleteHistoricById (no existe)", deleteNotFoundResponse);
}

