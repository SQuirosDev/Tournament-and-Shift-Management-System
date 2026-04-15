#include <sstream>
#include "logPetition.h"
#include "LogHistoric.h"
#include "factories.h"
#include "queuePetition.h"

LogPetition::LogPetition(Connection& dbConnection) : connection_(dbConnection) {
    logHistoric = nullptr;
}

void LogPetition::setLogHistoric(LogHistoric* historic) {
    logHistoric = historic;
}

BackendResponse LogPetition::insert(string requesterName, string type, string description) {

    if (requesterName.empty()) {
        return { -1, CODE_PETITION_INVALID_DATA, "El nombre del solicitante no puede estar vacío." };
    }

    if (type != "Inscripcion" && type != "Consulta" && type != "Apelacion") {
        return { -1, CODE_PETITION_INVALID_DATA, "Tipo de petición inválido. Use: Inscripcion, Consulta o Apelacion." };
    }

    BackendResponse response = dbResponseFactory(connection_.insertPetition(requesterName, type, description));

    if (response.code >= 4000 && response.code < 5000) {
        return response;
    }

    string newData = "{ \"id\": " + to_string(response.id) + ", \"requesterName\": \"" + requesterName + "\", \"type\": \"" + type + "\", \"description\": \"" + description + "\", \"response\": \"\", \"status\": \"Pendiente\", \"queuePosition\": 0 }";

    logHistoric->insert(historicFactory("Insert", "Petition", response.id, "{}", newData));

    return response;
}

BackendResponse LogPetition::update(int id, string responseText) {

    if (id <= 0) {
        return { -1, CODE_PETITION_INVALID_DATA, "El ID de la peticion no es válido." };
    }

    if (responseText.empty()) {
        return { -1, CODE_PETITION_INVALID_DATA, "La respuesta no puede estar vacía." };
    }

    BackendQueryResponse<Petition> queryResponse = dbQueryResponseFactory<Petition>(connection_.obtainPetitionById(id));

    if (queryResponse.data.empty()) {
        return { -1, CODE_PETITION_NOT_FOUND, "Peticion no encontrado." };
    }

    Petition petition = queryResponse.data[0];

    string previousData = "{ \"id\": " + to_string(petition.id) + ", \"requesterName\": \"" + petition.requesterName + "\", \"type\": \"" + petition.type + "\", \"description\": \"" + petition.description + "\", \"response\": \"" + petition.response + "\", \"status\": \"" + petition.status + "\", \"queuePosition\": " + to_string(petition.queuePosition) + " }";
    string newData = "{ \"id\": " + to_string(petition.id) + ", \"requesterName\": \"" + petition.requesterName + "\", \"type\": \"" + petition.type + "\", \"description\": \"" + petition.description + "\", \"response\": \"" + responseText + "\", \"status\": \"Atendida\", \"queuePosition\": " + to_string(petition.queuePosition) + " }";


    BackendResponse response = dbResponseFactory(connection_.updatePetitionStatus(id, responseText, "Atendida"));

    if (response.code >= 4000 && response.code < 5000) {
        return response;
    }

    logHistoric->insert(historicFactory("Update", "Petition", response.id, previousData, newData));

    return response;
}

BackendQueryResponse<Petition> LogPetition::peekNext() {

    BackendQueryResponse<Petition> response = dbQueryResponseFactory(connection_.listPendingPetitions());

    if (response.code >= 4000 && response.code < 5000 || response.data.empty()) {
        return { {}, CODE_PETITION_NOT_FOUND, "No hay solicitudes pendientes en la cola." };
    }

    // Crear cola
    QueuePetition petitionQueue;

    for (int i = 0; i < response.data.size(); i++) {
        petitionQueue.enqueue(response.data[i]);
    }

    // Peek (NO dequeue porque es peek)
    Petition nextPetition = petitionQueue.front();

    return { { nextPetition }, CODE_PETITION_LISTED, "Siguiente peticion obtenida correctamente." };
}

BackendQueryResponse<Petition> LogPetition::listPending() {

    return dbQueryResponseFactory(connection_.listPendingPetitions());
}

BackendResponse LogPetition::eliminar(int id)
{
    if (id <= 0) {
        return { -1, CODE_PETITION_INVALID_DATA, "El ID de la peticion no es válido." };
    }

    BackendQueryResponse<Petition> queryResponse = dbQueryResponseFactory<Petition>(connection_.obtainPetitionById(id));

    if (queryResponse.data.empty()) {
        return { -1, CODE_PETITION_NOT_FOUND, "Peticion no encontrado." };
    }

    Petition petition = queryResponse.data[0];

    string previousData = "{ \"id\": " + to_string(petition.id) + ", \"requesterName\": \"" + petition.requesterName + "\", \"type\": \"" + petition.type + "\", \"description\": \"" + petition.description + "\", \"response\": \"" + petition.response + "\", \"status\": \"" + petition.status + "\", \"queuePosition\": " + to_string(petition.queuePosition) + " }";

    BackendResponse response = dbResponseFactory(connection_.deletePetition(id));

    if (response.code >= 4000 && response.code < 5000) {
        return response;
    }

    logHistoric->insert(historicFactory("Delete", "Petition", response.id, previousData, "{}"));

    return response;
}

int LogPetition::pendingCount() {

    BackendQueryResponse<Petition> listResponse = dbQueryResponseFactory(connection_.listPendingPetitions());

    if (listResponse.code >= 4000 && listResponse.code < 5000) {
        return 0;
    }

    return static_cast<int>(listResponse.data.size());
}
