#include "logHistoric.h"
#include "logPlayer.h"
#include "logPetition.h"

LogHistoric::LogHistoric(Connection& dbConnection) : connection(dbConnection) {
    logPlayer = nullptr;
    logPetition = nullptr;
}

void LogHistoric::setLogPlayer(LogPlayer* player) {
    logPlayer = player;
}

void LogHistoric::setLogPetition(LogPetition* petition) {
    logPetition = petition;
}

BackendResponse LogHistoric::insert(Historic h) {
    return dbResponseFactory(connection.insertHistoric(h.actionType, h.entityType, h.recordId, h.previousData, h.newData, h.stackPosition));
}

BackendResponse LogHistoric::undo() {

    // Obtener historial
    DBQueryResponse<Historic> res = connection.listHistoric();

    if (res.data.empty()) {
        return {-1, 404, "No hay acciones para deshacer"};
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
    else {
        return {-1,  400, "Entidad no soportada" };
    }

    if (undoResponse.id == -1) {
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
        return { -1, CODE_HISTORIC_INVALID, "Error parseando JSON en previousData" };
    }

    if (h.actionType == "INSERT") {
        return logPlayer->eliminar(h.recordId);
    }
    else if (h.actionType == "UPDATE") {
        string name = prev.value("name", "");
        return logPlayer->update(h.recordId, name);
    }
    else if (h.actionType == "DELETE") 
    {
        string name = prev.value("name", "");
        return logPlayer->insert(0, name);
    }
    return {-1, CODE_HISTORIC_INVALID, "Accion no soportada para TEAM" };
}

BackendResponse LogHistoric::undoPetition(Historic& h) {
    json prev;

    try {
        prev = json::parse(h.previousData);
    }
    catch (...) {
        return { -1, CODE_HISTORIC_INVALID, "Error parseando JSON en previousData" };
    }

    if (h.actionType == "INSERT") {
        return logPetition->eliminar(h.recordId);
    }
    else if (h.actionType == "UPDATE") {
        string responseText = prev.value("response", "");
        return logPetition->update(h.recordId, responseText);
    }
    else if (h.actionType == "DELETE") {
        string requesterName = prev.value("requesterName", "");
        string type = prev.value("type", "");
        string description = prev.value("description", "");
        return logPetition->insert(requesterName, type, description);
    }

    return { -1, CODE_HISTORIC_INVALID, "Accion no soportada para PETITION" };
}
