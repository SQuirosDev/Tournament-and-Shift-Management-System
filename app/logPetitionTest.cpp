#include <QDebug>
#include <QCoreApplication>
#include "logPetitionTest.h"
#include "connection.h"
#include "logPetition.h"
#include "LogHistoric.h"

void logPetitionTest() {

    Connection connection;

    connection.open("test.db");

    LogPetition logPetition(connection);
    LogHistoric logHistoric(connection);

    logPetition.setLogHistoric(&logHistoric);

    qDebug() << "\n===== TEST LOG PETITION =====";

    // =========================
    // INSERT
    // =========================
    qDebug() << "\n-- INSERT PETITIONS --";

    BackendResponse r1 = logPetition.insert("Juan", "Consulta", "Pregunta sobre torneo");
    qDebug() << "Code:" << r1.code;
    qDebug() << "Message:" << QString::fromStdString(r1.message);
    qDebug() << "ID:" << r1.id;

    BackendResponse r2 = logPetition.insert("Maria", "Inscripcion", "Quiero participar");
    qDebug() << "Code:" << r2.code;
    qDebug() << "Message:" << QString::fromStdString(r2.message);
    qDebug() << "ID:" << r2.id;

    BackendResponse r3 = logPetition.insert("Carlos", "Apelacion", "Revisar resultado");
    qDebug() << "Code:" << r3.code;
    qDebug() << "Message:" << QString::fromStdString(r3.message);
    qDebug() << "ID:" << r3.id;


    // =========================
    // LIST PENDING
    // =========================
    qDebug() << "\n-- LIST PENDING --";

    BackendQueryResponse<Petition> listResponse = logPetition.listPending();

    for (Petition p : listResponse.data) {
        qDebug() << "ID:" << p.id
            << "Name:" << QString::fromStdString(p.requesterName)
            << "Type:" << QString::fromStdString(p.type);
    }

    // =========================
    // PEEK NEXT (COLA)
    // =========================
    qDebug() << "\n-- PEEK NEXT --";

    BackendQueryResponse<Petition> peekResponse = logPetition.peekNext();

    if (!peekResponse.data.empty()) {
        Petition p = peekResponse.data[0];
        qDebug() << "Next Petition ID:" << p.id
            << "Name:" << QString::fromStdString(p.requesterName);
    }

    // =========================
    // UPDATE (ATENDER)
    // =========================
    qDebug() << "\n-- UPDATE PETITION --";

    if (!listResponse.data.empty()) {
        int petitionId = listResponse.data[0].id;

        BackendResponse updateResponse = logPetition.update(petitionId, "Resuelto correctamente");

        qDebug() << "Code:" << updateResponse.code;
        qDebug() << "Message:" << QString::fromStdString(updateResponse.message);
    }

    // =========================
    // COUNT
    // =========================
    qDebug() << "\n-- COUNT PENDING --";

    int count = logPetition.pendingCount();
    qDebug() << "Pending Count:" << count;

    // =========================
    // DELETE
    // =========================
    qDebug() << "\n-- DELETE PETITION --";

    if (!listResponse.data.empty()) {
        int petitionId = listResponse.data[0].id;

        BackendResponse deleteResponse = logPetition.eliminar(petitionId);

        qDebug() << "Code:" << deleteResponse.code;
        qDebug() << "Message:" << QString::fromStdString(deleteResponse.message);
    }

    qDebug() << "\n===== END TEST =====";
}
