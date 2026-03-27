#include <sstream>
#include "logPetition.h"
#include "factories.h"

LogPetitions::LogPetitions(Connection& dbConnection) : connection_(dbConnection) {}

BackendResponse LogPetitions::registerPetition(string requesterName, string type, string description) {

    if (requesterName.empty()) {
        return { -1, CODE_PETITION_INVALID_DATA, "El nombre del solicitante no puede estar vacío." };
    }

    if (type != "Inscripcion" && type != "Consulta" && type != "Apelacion") {
        return { -1, CODE_PETITION_INVALID_DATA, "Tipo de petición inválido. Use: Inscripcion, Consulta o Apelacion." };
    }

    return dbResponseFactory(connection_.insertPetition(requesterName, type, description));
}

BackendResponse LogPetitions::updatePetition(int id, string responseText) {

    if (responseText.empty()) {
        return { -1, CODE_PETITION_INVALID_DATA, "La respuesta no puede estar vacía." };
    }

    DbResponse updateResponse = connection_.updatePetitionStatus(id, responseText, "Atendida");

    if (updateResponse.code != CODE_OK && updateResponse.code != CODE_UPDATE_OK) {
        return dbResponseFactory(updateResponse);
    }

    return { updateResponse.id, CODE_PETITION_ATTENDED, "Petición atendida correctamente." };
}

BackendQueryResponse<Petition> LogPetitions::peekNextPetition() {

    DBQueryResponse obtainResponse = connection_.obtainNextPetition();

    if (obtainResponse.code != CODE_OK && obtainResponse.code != CODE_SELECT_OK) {
        return { {}, CODE_PETITION_NOT_FOUND, "No hay solicitudes pendientes en la cola." };
    }

    return dbQueryResponseFactory(obtainResponse);
}

BackendQueryResponse<Petition> LogPetitions::listPendingPetitions() {

    return dbQueryResponseFactory(connection_.listPendingPetitions());
}

int LogPetitions::pendingCount() {

    BackendQueryResponse listResponse = dbQueryResponseFactory(connection_.listPendingPetitions());

    if (listResponse.code != CODE_OK && listResponse.code != CODE_PETITION_LISTED) {
        return 0;
    }

    return static_cast<int>(listResponse.data.size());
}
