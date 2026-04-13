#include <QDebug>
#include "logPlayerTest.h"
#include "connection.h"
#include "logPlayer.h"
#include "LogHistoric.h"

void logPlayerTest() {

    Connection connection;

    // Abrir DB
    DbResponse dbOpenResponse = connection.open("test.db");

    if (dbOpenResponse.code != CODE_SUCCESS) {
        qDebug() << "Error al abrir DB:" << QString::fromStdString(dbOpenResponse.message);
        return;
    }

    // Crear logicas
    LogPlayer logPlayer(connection);
    LogHistoric logHistoric(connection);

    logPlayer.setLogHistoric(&logHistoric);

    qDebug() << "\n===== TEST LOG PLAYER =====";

    // ⚠️ Debe existir un TEAM en DB
    int teamId = 1;

    // =========================
    // INSERT
    // =========================
    qDebug() << "\n-- INSERT PLAYER --";

    BackendResponse insertResponse = logPlayer.insert(teamId, "Messi");

    qDebug() << "Code:" << insertResponse.code;
    qDebug() << "Message:" << QString::fromStdString(insertResponse.message);
    qDebug() << "New ID:" << insertResponse.id;

    int playerId = insertResponse.id;

    // =========================
    // LIST BY TEAM
    // =========================
    qDebug() << "\n-- LIST PLAYERS BY TEAM --";

    BackendQueryResponse<Player> listResponse = logPlayer.listByTeam(teamId);

    qDebug() << "Code:" << listResponse.code;
    qDebug() << "Message:" << QString::fromStdString(listResponse.message);

    for (Player player : listResponse.data) {
        qDebug() << "ID:" << player.id
            << "Name:" << QString::fromStdString(player.name);
    }

    // =========================
    // OBTAIN BY ID
    // =========================
    qDebug() << "\n-- OBTAIN PLAYER BY ID --";

    BackendQueryResponse<Player> obtainResponse = logPlayer.obtainById(playerId);

    qDebug() << "Code:" << obtainResponse.code;
    qDebug() << "Message:" << QString::fromStdString(obtainResponse.message);

    if (!obtainResponse.data.empty()) {
        Player player = obtainResponse.data[0];
        qDebug() << "ID:" << player.id
            << "Name:" << QString::fromStdString(player.name);
    }

    // =========================
    // UPDATE
    // =========================
    qDebug() << "\n-- UPDATE PLAYER --";

    BackendResponse updateResponse = logPlayer.update(playerId, "Cristiano Ronaldo");

    qDebug() << "Code:" << updateResponse.code;
    qDebug() << "Message:" << QString::fromStdString(updateResponse.message);

    // =========================
    // DELETE
    // =========================
    qDebug() << "\n-- DELETE PLAYER --";

    BackendResponse deleteResponse = logPlayer.eliminar(playerId);

    qDebug() << "Code:" << deleteResponse.code;
    qDebug() << "Message:" << QString::fromStdString(deleteResponse.message);

    qDebug() << "\n===== END TEST =====";
}
