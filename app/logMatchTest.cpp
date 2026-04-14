#include <QDebug>
#include <QCoreApplication>
#include "logMatchTest.h"
#include "connection.h"
#include "logMatch.h"
#include "logTeam.h"
#include "logTournament.h"
#include "LogHistoric.h"
#include "factories.h"

void logMatchTest() {

    Connection connection;

    connection.open("test.db");

    LogMatch logMatch(connection);
    LogHistoric logHistoric(connection);

    logMatch.setLogHistoric(&logHistoric);

    qDebug() << "\n===== TEST LOG MATCH =====";

    // =========================
    // SETUP
    // =========================
    int tournamentId = 1;
    int teamAId = 1;
    int teamBId = 2;

    // =========================
    // INSERT MATCH
    // =========================
    qDebug() << "\n-- INSERT MATCH --";

    BackendResponse insertResponse = logMatch.insert(tournamentId, teamAId, teamBId);

    qDebug() << "Code:" << insertResponse.code;
    qDebug() << "Message:" << QString::fromStdString(insertResponse.message);
    qDebug() << "Match ID:" << insertResponse.id;

    int matchId = insertResponse.id;

    // =========================
    // LIST BY TOURNAMENT
    // =========================
    qDebug() << "\n-- LIST MATCHES BY TOURNAMENT --";

    BackendQueryResponse<Match> listResponse = logMatch.listByTournament(tournamentId);

    for (Match m : listResponse.data) {
        qDebug() << "ID:" << m.id
            << "Phase:" << QString::fromStdString(m.phase)
            << "Status:" << QString::fromStdString(m.status);
    }

    // =========================
    // UPDATE MATCH (FINALIZAR)
    // =========================
    qDebug() << "\n-- UPDATE MATCH (FINALIZAR) --";

    BackendResponse updateResponse = logMatch.update(
        teamAId,
        "Grupos",
        0,
        "Finalizado",
        teamAId, // gana A
        "2-1"
    );

    qDebug() << "Code:" << updateResponse.code;
    qDebug() << "Message:" << QString::fromStdString(updateResponse.message);

    // =========================
    // VALIDAR STATS
    // =========================
    qDebug() << "\n-- VALIDATE TEAM STATS --";

    BackendQueryResponse<Team> teamAData = dbQueryResponseFactory<Team>(connection.obtainTeamById(teamAId));
    BackendQueryResponse<Team> teamBData = dbQueryResponseFactory<Team>(connection.obtainTeamById(teamBId));

    if (!teamAData.data.empty()) {
        Team t = teamAData.data[0];
        qDebug() << "Team A Points:" << t.points << "Wins:" << t.wins;
    }

    if (!teamBData.data.empty()) {
        Team t = teamBData.data[0];
        qDebug() << "Team B Points:" << t.points << "Losses:" << t.losses;
    }

    // =========================
    // LIST BY PHASE
    // =========================
    qDebug() << "\n-- LIST BY PHASE --";

    BackendQueryResponse<Match> phaseList = logMatch.listByPhase(tournamentId, "Grupos");

    for (Match m : phaseList.data) {
        qDebug() << "Match ID:" << m.id;
    }

    // =========================
    // DELETE
    // =========================
    qDebug() << "\n-- DELETE MATCH --";

    BackendResponse deleteResponse = logMatch.eliminar(matchId);

    qDebug() << "Code:" << deleteResponse.code;
    qDebug() << "Message:" << QString::fromStdString(deleteResponse.message);

    qDebug() << "\n===== END TEST =====";
}
