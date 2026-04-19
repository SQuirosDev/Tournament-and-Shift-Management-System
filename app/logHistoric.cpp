#include <string>
#include <vector>
#include "logHistoric.h"
#include "factories.h"
#include "stackHistoric.h"
#include "codes.h"
#include "logPlayer.h"
#include "logPetition.h"
#include "logTeam.h"
#include "logTournament.h"
#include "logMatch.h"

LogHistoric::LogHistoric(Connection& dbConnection) : connection(dbConnection) {
    logPlayer = nullptr;
    logPetition = nullptr;
    logTeam = nullptr;
    logTournament = nullptr;
    logMatch = nullptr;
}

void LogHistoric::setLogPlayer(LogPlayer* player) {
    logPlayer = player;
}

void LogHistoric::setLogPetition(LogPetition* petition) {
    logPetition = petition;
}

void LogHistoric::setLogTeam(LogTeam* team) {
    logTeam = team;
}

void LogHistoric::setLogTournament(LogTournament* tournament) {
    logTournament = tournament;
}

void LogHistoric::setLogMatch(LogMatch* match) {
    logMatch = match;
}

BackendResponse LogHistoric::insert(Historic h) {
    return dbResponseFactory(connection.insertHistoric(h.actionType, h.entityType, h.recordId, h.previousData, h.newData, h.stackPosition));
}

BackendResponse LogHistoric::undo() {

    // Obtener historial
    DBQueryResponse<Historic> res = connection.listHistoric();

    if (res.data.empty()) {
        return {-1, CODE_HISTORIC_EMPTY, "No hay acciones para deshacer"};
    }

    // Crear pila
    StackHistoric pila;

    for (auto& h : res.data) {
        pila.push(h);
    }
    
    // Obtener último
    Historic last = pila.top();

    // Buscar por entidad
    BackendResponse undoResponse;

    if (last.entityType == "Player") {
        undoResponse = undoPlayer(last);
    }
    else if (last.entityType == "Petition") {
        undoResponse = undoPetition(last);
    }
    else if (last.entityType == "Team") {
        undoResponse = undoTeam(last);
    }
    else if (last.entityType == "Tournament") {
        undoResponse = undoTournament(last);
    }
    else if (last.entityType == "Match") {
        undoResponse = undoMatch(last);
    }
    else {
        return {-1,  CODE_UNDO_NOT_AVAILABLE, "Entidad no soportada" };
    }

    if (undoResponse.code >= 4000 && undoResponse.code < 5000) {
        return undoResponse;
    }

    // Eliminar de BD
    return dbResponseFactory(connection.deleteLastHistoric());
}

BackendResponse LogHistoric::undoPlayer(Historic& h) {
    json prev;

    try {
        prev = json::parse(h.previousData);
    }
    catch (...) {
        return { -1, CODE_HISTORIC_JSON_ERROR, "Error parseando JSON en previousData" };
    }

    if (h.actionType == "Insert") {
        return logPlayer->eliminar(h.recordId);
    }
    else if (h.actionType == "Update") {
        string name = prev.value("name", "");
        return logPlayer->update(h.recordId, name);
    }
    else if (h.actionType == "Delete") 
    {
        string name = prev.value("name", "");
        return logPlayer->insert(0, name);
    }
    return {-1, CODE_HISTORIC_INVALID_DATA, "Accion no soportada para TEAM" };
}

BackendResponse LogHistoric::undoPetition(Historic& h) {
    json prev;

    try {
        prev = json::parse(h.previousData);
    }
    catch (...) {
        return { -1, CODE_HISTORIC_JSON_ERROR, "Error parseando JSON en previousData" };
    }

    if (h.actionType == "Insert") {
        return logPetition->eliminar(h.recordId);
    }
    else if (h.actionType == "Update") {
        string responseText = prev.value("response", "");
        return logPetition->update(h.recordId, responseText);
    }
    else if (h.actionType == "Delete") {
        string requesterName = prev.value("requesterName", "");
        string type = prev.value("type", "");
        string description = prev.value("description", "");
        return logPetition->insert(requesterName, type, description);
    }

    return { -1, CODE_HISTORIC_INVALID_DATA, "Accion no soportada para PETITION" };
}

BackendResponse LogHistoric::undoTeam(Historic& h) {
    json prev;

    try {
        prev = json::parse(h.previousData);
    }
    catch (...) {
        return { -1, CODE_HISTORIC_JSON_ERROR, "Error parseando JSON en previousData" };
    }
    
    if (h.actionType == "Insert") {
        return logTeam->eliminar(h.recordId);
    }
    else if (h.actionType == "Update") {
        string name = prev.value("name", "");
        return logTeam->update(h.recordId, name);
    }
    else if (h.actionType == "Delete") {
        string name = prev.value("name", "");
        string tournamentId = prev.value("tournamentId", "");
        return logTeam->insert(name, stoi(tournamentId) );
    }

    return { -1, CODE_HISTORIC_INVALID_DATA, "Accion no soportada para PETITION" };
}

BackendResponse LogHistoric::undoTournament(Historic& h) {
    json prev;

    try {
        prev = json::parse(h.previousData);
    }
    catch (...) {
        return { -1, CODE_HISTORIC_JSON_ERROR, "Error parseando JSON en previousData" };
    }

    if (h.actionType == "Insert") {
        return logTournament->eliminar(h.recordId);
    }
    else if (h.actionType == "Update") {
        string name = prev.value("name", "");
        return logTournament->updateName(h.recordId, name);
    }
    else if (h.actionType == "Delete") {
        string name = prev.value("name", "");
        return logTournament->insert(name);
    }

    return { -1, CODE_HISTORIC_INVALID_DATA, "Accion no soportada para PETITION" };
}

BackendResponse LogHistoric::undoMatch(Historic& h) {
    json prev;

    try {
        prev = json::parse(h.previousData);
    }
    catch (...) {
        return { -1, CODE_HISTORIC_JSON_ERROR, "Error parseando JSON en previousData" };
    }

    if (h.actionType == "Insert") {
        return logMatch->eliminar(h.recordId);
    }
    else if (h.actionType == "Update") {
        string phase = prev.value("phase", "");
        string round = prev.value("round", "");
        string status = prev.value("status", "");
        string winnerId = prev.value("winnerId", "");
        string result = prev.value("result", "");
        return logMatch->update(h.recordId, phase, stoi(round), status, stoi(winnerId), result);
    }
    else if (h.actionType == "Delete") {
        string tournamentId = prev.value("tournamentId", "");
        string teamAId = prev.value("teamAId", "");
        string teamBId = prev.value("teamBId", "");
        return logMatch->insert(stoi(tournamentId), stoi(teamAId), stoi(teamBId));
    }

    return { -1, CODE_HISTORIC_INVALID_DATA, "Accion no soportada para PETITION" };
}
