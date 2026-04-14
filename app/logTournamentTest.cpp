#include <QDebug>
#include <QCoreApplication>
#include "logTournamentTest.h"
#include "connection.h"
#include "logTournament.h"
#include "LogHistoric.h"

void logTournamentTest() {

    Connection connection;

    connection.open("test.db");

    LogTournament logTournament(connection);
    LogHistoric logHistoric(connection);

    logTournament.setLogHistoric(&logHistoric);

    qDebug() << "\n===== TEST LOG TOURNAMENT =====";

    // =========================
    // INSERT
    // =========================
    qDebug() << "\n-- INSERT TOURNAMENT --";

    BackendResponse insertResponse = logTournament.insert("Champions League");

    qDebug() << "Code:" << insertResponse.code;
    qDebug() << "Message:" << QString::fromStdString(insertResponse.message);
    qDebug() << "Tournament ID:" << insertResponse.id;

    int tournamentId = insertResponse.id;

    // =========================
    // LIST
    // =========================
    qDebug() << "\n-- LIST TOURNAMENTS --";

    BackendQueryResponse<Tournament> listResponse = logTournament.list();

    qDebug() << "Code:" << listResponse.code;
    qDebug() << "Message:" << QString::fromStdString(listResponse.message);

    for (Tournament t : listResponse.data) {
        qDebug() << "ID:" << t.id
            << "Name:" << QString::fromStdString(t.name)
            << "Phase:" << QString::fromStdString(t.phase);
    }

    // =========================
    // OBTAIN
    // =========================
    qDebug() << "\n-- OBTAIN TOURNAMENT --";

    BackendQueryResponse<Tournament> obtainResponse = logTournament.obtain(tournamentId);

    qDebug() << "Code:" << obtainResponse.code;
    qDebug() << "Message:" << QString::fromStdString(obtainResponse.message);

    if (!obtainResponse.data.empty()) {
        Tournament t = obtainResponse.data[0];
        qDebug() << "ID:" << t.id
            << "Name:" << QString::fromStdString(t.name)
            << "Phase:" << QString::fromStdString(t.phase);
    }

    // =========================
    // UPDATE NAME
    // =========================
    qDebug() << "\n-- UPDATE NAME --";

    BackendResponse updateNameResponse = logTournament.updateName(tournamentId, "Champions PRO");

    qDebug() << "Code:" << updateNameResponse.code;
    qDebug() << "Message:" << QString::fromStdString(updateNameResponse.message);

    // =========================
    // UPDATE PHASE
    // =========================
    qDebug() << "\n-- UPDATE PHASE --";

    BackendResponse updatePhase1 = logTournament.updatePhase(tournamentId, "Grupos");
    qDebug() << "To Grupos:" << updatePhase1.code << QString::fromStdString(updatePhase1.message);

    BackendResponse updatePhase2 = logTournament.updatePhase(tournamentId, "Eliminacion");
    qDebug() << "To Eliminacion:" << updatePhase2.code << QString::fromStdString(updatePhase2.message);

    BackendResponse updatePhase3 = logTournament.updatePhase(tournamentId, "Finalizado");
    qDebug() << "To Finalizado:" << updatePhase3.code << QString::fromStdString(updatePhase3.message);

    // =========================
    // TRY INVALID UPDATE
    // =========================
    qDebug() << "\n-- INVALID PHASE (BACK) --";

    BackendResponse invalidPhase = logTournament.updatePhase(tournamentId, "Registro");

    qDebug() << "Code:" << invalidPhase.code;
    qDebug() << "Message:" << QString::fromStdString(invalidPhase.message);

    // =========================
    // DELETE
    // =========================
    qDebug() << "\n-- DELETE TOURNAMENT --";

    BackendResponse deleteResponse = logTournament.eliminar(tournamentId);

    qDebug() << "Code:" << deleteResponse.code;
    qDebug() << "Message:" << QString::fromStdString(deleteResponse.message);

    qDebug() << "\n===== END TEST =====";
}
