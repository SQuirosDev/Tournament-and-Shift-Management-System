#include <QDebug>
#include "logTeamTest.h"
#include "connection.h"
#include "logTeam.h"
#include "LogHistoric.h"

void logTeamTest() {

    Connection connection;

    connection.open("test.db");

    LogTeam logTeam(connection);
    LogHistoric logHistoric(connection);

    logTeam.setLogHistoric(&logHistoric);

    qDebug() << "\n===== TEST LOG TEAM =====";

    // =========================
    // CREAR TORNEO BASE
    // =========================
    int tournamentId = 1;

    qDebug() << "Tournament ID:" << tournamentId;

    // =========================
    // INSERT
    // =========================
    qDebug() << "\n-- INSERT TEAM --";

    BackendResponse insertResponse = logTeam.insert("Real Madrid", tournamentId);

    qDebug() << "Code:" << insertResponse.code;
    qDebug() << "Message:" << QString::fromStdString(insertResponse.message);
    qDebug() << "Team ID:" << insertResponse.id;

    int teamId = insertResponse.id;

    // =========================
    // LIST
    // =========================
    qDebug() << "\n-- LIST TEAMS --";

    BackendQueryResponse<Team> listResponse = logTeam.list();

    qDebug() << "Code:" << listResponse.code;
    qDebug() << "Message:" << QString::fromStdString(listResponse.message);

    for (Team team : listResponse.data) {
        qDebug() << "ID:" << team.id
            << "Name:" << QString::fromStdString(team.name)
            << "Points:" << team.points;
    }

    // =========================
    // UPDATE
    // =========================
    qDebug() << "\n-- UPDATE TEAM --";

    BackendResponse updateResponse = logTeam.update(teamId, "Manchester City");

    qDebug() << "Code:" << updateResponse.code;
    qDebug() << "Message:" << QString::fromStdString(updateResponse.message);

    // =========================
    // DELETE
    // =========================
    qDebug() << "\n-- DELETE TEAM --";

    BackendResponse deleteResponse = logTeam.eliminar(teamId);

    qDebug() << "Code:" << deleteResponse.code;
    qDebug() << "Message:" << QString::fromStdString(deleteResponse.message);

    qDebug() << "\n===== END TEST =====";
}
