#include <QDebug>
#include <QCoreApplication>
#include "logHistoricTest.h"
#include "connection.h"
#include "logHistoric.h"
#include "logTournament.h"

void logHistoricTest() {

    Connection connection;

    connection.open("test.db");

    LogHistoric logHistoric(connection);
    LogTournament logTournament(connection);

    // Conectar dependencias
    logTournament.setLogHistoric(&logHistoric);
    logHistoric.setLogTournament(&logTournament);

    qDebug() << "\n===== TEST LOG HISTORIC =====";

    // =========================
    // INSERT TOURNAMENT
    // =========================
    qDebug() << "\n-- INSERT TOURNAMENT --";

    BackendResponse insertResponse = logTournament.insert("Copa Test");

    qDebug() << "Code:" << insertResponse.code;
    qDebug() << "Message:" << QString::fromStdString(insertResponse.message);
    qDebug() << "Tournament ID:" << insertResponse.id;

    // =========================
    // VERIFY EXISTS
    // =========================
    qDebug() << "\n-- VERIFY INSERT --";

    auto listBefore = logTournament.list();

    for (auto t : listBefore.data) {
        qDebug() << "ID:" << t.id
            << "Name:" << QString::fromStdString(t.name);
    }

    // =========================
    // UNDO
    // =========================
    qDebug() << "\n-- UNDO --";

    BackendResponse undoResponse = logHistoric.undo();

    qDebug() << "Code:" << undoResponse.code;
    qDebug() << "Message:" << QString::fromStdString(undoResponse.message);

    // =========================
    // VERIFY DELETE
    // =========================
    qDebug() << "\n-- VERIFY AFTER UNDO --";

    auto listAfter = logTournament.list();

    for (auto t : listAfter.data) {
        qDebug() << "ID:" << t.id
            << "Name:" << QString::fromStdString(t.name);
    }

    qDebug() << "\n===== END TEST =====";
}
