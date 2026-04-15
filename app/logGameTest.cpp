#include <QDebug>
#include <QCoreApplication>
#include "logGameTest.h"
#include "connection.h"
#include "logGame.h"
#include "logTeam.h"
#include "logTournament.h"
#include "LogHistoric.h"

void logGameTest() {

    Connection connection;

    connection.open("test.db");

    LogGame logGame(connection);
    LogTeam logTeam(connection);
    LogTournament logTournament(connection);
    LogHistoric logHistoric(connection);

    logTournament.setLogHistoric(&logHistoric);
    logTeam.setLogHistoric(&logHistoric);

    qDebug() << "\n===== TEST LOG GAME =====";

    // =========================
    // SETUP
    // =========================
    qDebug() << "\n-- SETUP --";

    //BackendResponse tRes = logTournament.insert("Torneo Game");
    int tournamentId = 6;

    // Crear 8 equipos
    for (int i = 0; i < 8; i++) {
        logTeam.insert("Team_" + to_string(i + 1), tournamentId);
    }

    // =========================
    // GENERATE GROUP MATCHES
    // =========================
    qDebug() << "\n-- GENERATE GROUP MATCHES --";

    BackendResponse genGroups = logGame.generateGroupMatches(tournamentId);

    qDebug() << "Code:" << genGroups.code;
    qDebug() << "Message:" << QString::fromStdString(genGroups.message);

    // =========================
    // PLAY GROUP MATCHES
    // =========================
    qDebug() << "\n-- PLAY GROUP MATCHES --";

    BackendResponse playGroups = logGame.playGroupMatches(tournamentId);

    qDebug() << "Code:" << playGroups.code;
    qDebug() << "Message:" << QString::fromStdString(playGroups.message);

    // =========================
    // GENERATE SEMIS
    // =========================
    qDebug() << "\n-- GENERATE SEMIS --";

    BackendResponse genSemis = logGame.generateSemiMatches(tournamentId);

    qDebug() << "Code:" << genSemis.code;
    qDebug() << "Message:" << QString::fromStdString(genSemis.message);

    // =========================
    // PLAY SEMIS
    // =========================
    qDebug() << "\n-- PLAY SEMIS --";

    BackendResponse playSemis = logGame.playSemiMatches(tournamentId);

    qDebug() << "Code:" << playSemis.code;
    qDebug() << "Message:" << QString::fromStdString(playSemis.message);

    // =========================
    // GENERATE FINAL
    // =========================
    qDebug() << "\n-- GENERATE FINAL --";

    BackendResponse genFinal = logGame.generateFinalMatch(tournamentId);

    qDebug() << "Code:" << genFinal.code;
    qDebug() << "Message:" << QString::fromStdString(genFinal.message);

    // =========================
    // PLAY FINAL
    // =========================
    qDebug() << "\n-- PLAY FINAL --";

    BackendResponse playFinal = logGame.playFinalMatch(tournamentId);

    qDebug() << "Code:" << playFinal.code;
    qDebug() << "Message:" << QString::fromStdString(playFinal.message);

    qDebug() << "\n===== END TEST =====";
}
