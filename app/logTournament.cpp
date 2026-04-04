#include <vector>
#include "logTournament.h"
#include "LogHistoric.h"
#include "factories.h"
#include "validations.h"
#include "Tournaments.h"

LogTournament::LogTournament(Connection& dbConnection) : connection_(dbConnection) {
    logHistoric = nullptr;
}

void LogTournament::setLogHistoric(LogHistoric* historic) {
    logHistoric = historic;
}

BackendResponse LogTournament::insert(string name) {

    if (!isValidName(name)) {
        return { -1, CODE_TOURNAMENT_INVALID_DATA, "El nombre del torneo no puede estar vacío." };
    }

    BackendResponse response = dbResponseFactory(connection_.insertTournament(name));

    if (response.code >= 4000 && response.code < 5000) {
        return response;
    }

    string newData = "{ \"id\": " + to_string(response.id) + ", \"name\": \"" + name + "\", \"phase\": \"Registro\" }";

    logHistoric->insert(historicFactory("Insert", "Tournament", response.id, "{}", newData));

    return response;
}

BackendQueryResponse<Tournament> LogTournament::list() {
    BackendQueryResponse<Tournament> response = dbQueryResponseFactory<Tournament>(connection_.listTournaments());
    return response;
}

BackendQueryResponse<Tournament> LogTournament::obtain(int id) {

    if (id <= 0) {
        return { {}, CODE_TOURNAMENT_NOT_FOUND, "El ID del torneo no es válido." };
    }

    BackendQueryResponse<Tournament> response = dbQueryResponseFactory<Tournament>(connection_.obtainTournamentById(id));
    return response;
}

BackendResponse LogTournament::updateName(int id, string newName) {

    if (id <= 0) {
        return { -1, CODE_TOURNAMENT_NOT_FOUND, "El ID del torneo no es válido." };
    }

    if (!isValidName(newName)) {
        return { -1, CODE_TOURNAMENT_INVALID_DATA, "El nombre del torneo no puede estar vacío." };
    }

    BackendQueryResponse<Tournament> queryResponse = dbQueryResponseFactory<Tournament>(connection_.obtainTournamentById(id));

    if (queryResponse.data.empty()) {
        return { -1, CODE_TOURNAMENT_NOT_FOUND, "torneo no encontrado." };
    }

    Tournament tournament = queryResponse.data[0];

    if (tournament.phase == "Finalizado") {
        return { -1, CODE_TOURNAMENT_INVALID_DATA, "No se puede modificar un torneo finalizado" };
    }

    if (tournament.phase != "Registro") {
        return { -1, CODE_TOURNAMENT_INVALID_DATA, "Solo se puede modificar el nombre en fase Registro" };
    }

    string previousData = "{ \"id\": " + to_string(tournament.id) + ", \"name\": \"" + tournament.name + "\", \"phase\": \"Registro\" }";
    string newData = "{ \"id\": " + to_string(tournament.id) + ", \"name\": \"" + newName + "\", \"phase\": \"Registro\" }";


    BackendResponse response = dbResponseFactory(connection_.updateTournamentName(id, newName));

    if (response.code >= 4000 && response.code < 5000) {
        return response;
    }

    logHistoric->insert(historicFactory("Update", "Tournament", response.id, previousData, newData));

    return response;
}

BackendResponse LogTournament::updatePhase(int id, string phase) {

    if (id <= 0) {
        return { -1, CODE_TOURNAMENT_NOT_FOUND, "El ID del torneo no es válido." };
    }

    if (!isValidName(phase)) {
        return { -1, CODE_TOURNAMENT_INVALID_DATA, "La fase del torneo no puede estar vacío." };
    }

    BackendQueryResponse<Tournament> queryResponse = dbQueryResponseFactory<Tournament>(connection_.obtainTournamentById(id));
   
    if (queryResponse.data.empty()) {
        return { -1, CODE_TEAM_NOT_FOUND, "torneo no encontrado." };
    }

    Tournament tournament = queryResponse.data[0];

    if (tournament.phase == "Finalizado") {
        return { -1, CODE_TOURNAMENT_INVALID_DATA, "El torneo ya esta finalizado" };
    }

    vector<string> validPhases = { "Registro", "Grupos", "Eliminacion", "Finalizado" };

    int currentIndex = -1;
    int newIndex = -1;

    for (int i = 0; i < validPhases.size(); i++) {
        if (tournament.phase == validPhases[i]) {
            currentIndex = i;
        }
        if (validPhases[i] == phase) {
            newIndex = i;
        }
    }

    if (newIndex == -1) {
        return { -1, CODE_TOURNAMENT_INVALID_DATA, "Fase invalida" };
    }

    if (newIndex < currentIndex) {
        return { -1, CODE_TOURNAMENT_INVALID_DATA, "No se puede retroceder la fase del torneo" };
    }

    BackendResponse response = dbResponseFactory(connection_.updateTournamentPhase(id, phase));

    return response;
}

BackendResponse LogTournament::eliminar(int id) {

    if (id <= 0) {
        return { -1, CODE_TOURNAMENT_NOT_FOUND, "El ID del toneo no es válido." };
    }

    BackendQueryResponse<Tournament> queryResponse = dbQueryResponseFactory<Tournament>(connection_.obtainTournamentById(id));

    if (queryResponse.data.empty()) {
        return { -1, CODE_TOURNAMENT_INVALID_DATA, "Torneo no encontrado." };
    }

    Tournament tournament = queryResponse.data[0];

    string previousData = "{ \"id\": " + to_string(tournament.id) + ", \"name\": \"" + tournament.name + "\", \"phase\": \"" + tournament.phase + "\" }";

    BackendResponse response = dbResponseFactory(connection_.deleteTeam(id));

    if (response.code >= 4000 && response.code < 5000) {
        return response;
    }

    logHistoric->insert(historicFactory("Delete", "Team", response.id, previousData, "{}"));

    return response;
}