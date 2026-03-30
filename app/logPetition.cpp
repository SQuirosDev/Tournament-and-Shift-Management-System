#include <sstream>
#include "logPetition.h"
#include "LogHistoric.h"
#include "factories.h"

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

    if (response.id <= 0) {
        return response;
    }

    string newData = "{ \"id\": " + to_string(response.id) + ", \"requesterName\": \"" + requesterName + "\", \"type\": \"" + type + "\", \"description\": \"" + description + "\", \"response\": \"" "\", \"status\": \"" "\", \"queuePosition\": " " }";

    logHistoric->insert(historicFactory("Insert", "Petition", response.id, "{}", newData));

    return response;
}

BackendResponse LogPetition::update(int id, string responseText) {

    if (id <= 0) {
        return { -1, 400, "El ID del jugador no es válido." };
    }

    if (responseText.empty()) {
        return { -1, CODE_PETITION_INVALID_DATA, "La respuesta no puede estar vacía." };
    }

    BackendQueryResponse<Petition> queryResponse = dbQueryResponseFactory<Petition>(connection_.obtainPetitionById(id));

    if (queryResponse.data.empty()) {
        return { -1, 404, "Jugador no encontrado." };
    }

    Petition petition = queryResponse.data[0];

    string previousData = "{ \"id\": " + to_string(petition.id) + ", \"requesterName\": \"" + petition.requesterName + "\", \"type\": \"" + petition.type + "\", \"description\": \"" + petition.description + "\", \"response\": \"" "\", \"status\": \"" "\", \"queuePosition\": " " }";
    string newData = "{ \"id\": " + to_string(petition.id) + ", \"requesterName\": \"" + petition.requesterName + "\", \"type\": \"" + petition.type + "\", \"description\": \"" + petition.description + "\", \"response\": \"" "\", \"status\": \"" "\", \"queuePosition\": " " }";


    BackendResponse response = dbResponseFactory(connection_.updatePetitionStatus(id, responseText, "Atendida"));

    if (response.id <= 0) {
        return response;
    }

    logHistoric->insert(historicFactory("Update", "Petition", response.id, previousData, newData));

    return response;
}

BackendQueryResponse<Petition> LogPetition::peekNextPetition() {

    DBQueryResponse obtainResponse = connection_.obtainNextPetition();

    if (obtainResponse.code != CODE_OK && obtainResponse.code != CODE_SELECT_OK) {
        return { {}, CODE_PETITION_NOT_FOUND, "No hay solicitudes pendientes en la cola." };
    }

    return dbQueryResponseFactory(obtainResponse);
}

BackendQueryResponse<Petition> LogPetition::listPendingPetitions() {

    return dbQueryResponseFactory(connection_.listPendingPetitions());
}

BackendResponse LogPetition::eliminar(int id)
{
    if (id <= 0) {
        return { -1, 400, "El ID del jugador no es válido." };
    }

    BackendQueryResponse<Petition> queryResponse = dbQueryResponseFactory<Petition>(connection_.obtainPetitionById(id));

    if (queryResponse.data.empty()) {
        return { -1, 404, "Jugador no encontrado." };
    }

    Petition petition = queryResponse.data[0];

    string previousData = "{ \"id\": " + to_string(petition.id) + ", \"requesterName\": \"" + petition.requesterName + "\", \"type\": \"" + petition.type + "\", \"description\": \"" + petition.description + "\", \"response\": \"" "\", \"status\": \"" "\", \"queuePosition\": " " }";

    BackendResponse response = dbResponseFactory(connection_.deletePetition(id));

    if (response.id <= 0) {
        return response;
    }

    logHistoric->insert(historicFactory("Delete", "Player", response.id, previousData, "{}"));

    return response;
}

int LogPetition::pendingCount() {

    BackendQueryResponse listResponse = dbQueryResponseFactory(connection_.listPendingPetitions());

    if (listResponse.code != CODE_OK && listResponse.code != CODE_PETITION_LISTED) {
        return 0;
    }

    return static_cast<int>(listResponse.data.size());
}
