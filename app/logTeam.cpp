#include <random>
#include <vector>
#include <algorithm>
#include "logTeam.h"
#include "LogHistoric.h"
#include "validations.h"
#include "factories.h"

LogTeam::LogTeam(Connection& dbConnection) : connection_(dbConnection) {
    logHistoric = nullptr;
}

void LogTeam::setLogHistoric(LogHistoric* historic) {
    logHistoric = historic;
}

BackendResponse LogTeam::insert(string name, int tournamentId) {

    if (tournamentId <= 0) {
        return { -1, CODE_TEAM_NOT_FOUND, "El ID del torneo no es válido." };
    }

    if (!isValidName(name)) {
        return { -1, CODE_TEAM_INVALID_DATA, "El nombre del equipo no puede estar vacío."};
    }

    BackendResponse response = dbResponseFactory(connection_.insertTeam(name, tournamentId));

    if (response.code >= 4000 && response.code < 5000) {
        return response;
    }

    string newData = "{ \"id\": " + to_string(response.id) + ", \"tournamentId\": " + to_string(0) + ", \"name\": \"" + name + "\", \"tournaments\": " + to_string(0) + ", \"points\": " + to_string(0) + ", \"wins\": " + to_string(0) + ", \"draws\": " + to_string(0) + ", \"losses\": " + to_string(0) + " }";

    // Historial
    logHistoric->insert(historicFactory("Insert", "Team", response.id, "{}", newData));

    return response;
}

BackendQueryResponse<Team> LogTeam::list() {
    BackendQueryResponse<Team> response = dbQueryResponseFactory<Team>(connection_.listAllTeams());
    return response;
}

BackendResponse LogTeam::update(int id, string newName) {

    if (id <= 0) {
        return { -1, CODE_TEAM_NOT_FOUND, "El ID del equipo no es válido." };
    }

    if (!isValidName(newName)) {
        return { -1, CODE_TEAM_INVALID_DATA, "El nombre del equipo no puede estar vacío." };
    }

    BackendQueryResponse<Team> queryResponse = dbQueryResponseFactory<Team>(connection_.obtainTeamById(id));

    if (queryResponse.data.empty()) {
        return { -1, CODE_TEAM_NOT_FOUND, "Equipo no encontrado." };
    }

    Team team = queryResponse.data[0];

    string previousData = "{ \"id\": " + to_string(team.id) + ", \"tournamentId\": " + to_string(team.tournamentId) + ", \"name\": \"" + team.name + "\", \"tournaments\": " + to_string(team.tournaments) + ", \"points\": " + to_string(team.points) + ", \"wins\": " + to_string(team.wins) + ", \"draws\": " + to_string(team.draws) + ", \"losses\": " + to_string(team.losses) + " }";
    string newData = "{ \"id\": " + to_string(team.id) + ", \"tournamentId\": " + to_string(team.tournamentId) + ", \"name\": \"" + newName + "\", \"tournaments\": " + to_string(team.tournaments) + ", \"points\": " + to_string(team.points) + ", \"wins\": " + to_string(team.wins) + ", \"draws\": " + to_string(team.draws) + ", \"losses\": " + to_string(team.losses) + " }";

    BackendResponse response = dbResponseFactory(connection_.updateTeam(id, newName));

    if (response.code >= 4000 && response.code < 5000) {
        return response;
    }

    logHistoric->insert(historicFactory("Update", "Team", response.id, previousData, newData));

    return response;
}

BackendResponse LogTeam::eliminar(int id) {

    if (id <= 0) {
        return { -1, CODE_TEAM_NOT_FOUND, "El ID del equipo no es válido." };
    }

    BackendQueryResponse<Team> queryResponse = dbQueryResponseFactory<Team>(connection_.obtainTeamById(id));

    if (queryResponse.data.empty()) {
        return { -1, CODE_TEAM_INVALID_DATA, "Equipo no encontrado." };
    }

    Team team = queryResponse.data[0];

    string previousData = "{ \"id\": " + to_string(team.id) + ", \"tournamentId\": " + to_string(team.tournamentId) + ", \"name\": \"" + team.name + "\", \"tournaments\": " + to_string(team.tournaments) + ", \"points\": " + to_string(team.points) + ", \"wins\": " + to_string(team.wins) + ", \"draws\": " + to_string(team.draws) + ", \"losses\": " + to_string(team.losses) + " }";


    BackendResponse response = dbResponseFactory(connection_.deleteTeam(id));

    if (response.code >= 4000 && response.code < 5000) {
        return response;
    }

    logHistoric->insert(historicFactory("Delete", "Team", response.id, previousData, "{}"));

    return response;
}

BackendResponse LogTeam::generate(int tournamentId) {
    // Obtener jugadores
    BackendQueryResponse playersResponse = dbQueryResponseFactory(connection_.listAllPlayers());
    if (playersResponse.code >= 4000 && playersResponse.code < 5000) {
        return { -1, playersResponse.code, playersResponse.message };
    }

    vector<Player> playersList = playersResponse.data;
    if (playersList.size() < 32) {
        return { -1, CODE_TEAM_INVALID_DATA, "Se requieren minimo 32 jugadores" };
    }

    // Obtener equipos
    BackendQueryResponse teamsResponse = dbQueryResponseFactory(connection_.listTeamsByTournament(tournamentId));
    if (teamsResponse.code >= 4000 && teamsResponse.code < 5000) {
        return { -1, teamsResponse.code, teamsResponse.message };
    }

    vector<Team> teamsList = teamsResponse.data;
    if (teamsList.size() < 8) {
        return { -1, CODE_TEAM_INVALID_DATA, "Se requieren minimo 8 equipos" };
    }

    // Mezclar jugadores aleatoriamente
    // Genera una semilla aleatoria desde el sistema
    random_device rd;
    // Inicializa el generador de numeros aleatorios con la semilla
    mt19937 generator(rd());
    // Mezcla aleatoriamente el vector de jugadores
    shuffle(playersList.begin(), playersList.end(), generator);

    int teamIndex = 0;
    int maxPlayersPerTeam = 4;

    for (int i = 0; i < playersList.size(); i++) {
        if (teamIndex >= teamsList.size()) {
            teamIndex = 0;
        }

        int playersAssigned = i % maxPlayersPerTeam;

        if (playersAssigned == 0 && i != 0) {
            teamIndex++;
            if (teamIndex >= teamsList.size()) {
                teamIndex = 0;
            }
        }

        int playerId = playersList[i].id;
        int teamId = teamsList[teamIndex].id;

        connection_.updatePlayerTeam(playerId, teamId);
    }

    return { 1, CODE_TEAM_GENERATE, "Equipos generados correctamente" };
}
