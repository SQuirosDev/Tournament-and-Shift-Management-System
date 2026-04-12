#include "connection.h"
#include <iostream>
#include <functional>

using namespace std;

// Constructores estaticos para crear parametros de cada tipo
static SqlParam paramInt(int value) {
    SqlParam parameter;
    parameter.isText = false;
    parameter.intValue = value;
    return parameter;
}

static SqlParam paramText(string value) {
    SqlParam parameter;
    parameter.isText = true;
    parameter.textValue = value;
    return parameter;
}

static bool bindParams(sqlite3_stmt* sqlStatement, vector<SqlParam> params) {
    for (int paramIndex = 0; paramIndex < (int)params.size(); paramIndex++) {
        int bindIndex = paramIndex + 1; // SQLite usa indices 1-based
        SqlParam currentParam = params[paramIndex];

        if (currentParam.isText) {
            // SQLITE_TRANSIENT hace una copia interna del string
            int bindResult = sqlite3_bind_text(sqlStatement, bindIndex, currentParam.textValue.c_str(), -1, SQLITE_TRANSIENT);
            if (bindResult != SQLITE_OK) {
                return false;
            }
        }
        else {
            int bindResult = sqlite3_bind_int(sqlStatement, bindIndex, currentParam.intValue);
            if (bindResult != SQLITE_OK) {
                return false;
            }
        }
    }
    return true;
}

static bool execSimple(sqlite3* dbConnection, const char* sqlQuery) {
    char* errorMessage = nullptr;
    int resultCode = sqlite3_exec(dbConnection, sqlQuery, nullptr, nullptr, &errorMessage);
    if (errorMessage) {
        sqlite3_free(errorMessage);
    }
    return resultCode == SQLITE_OK;
}

static bool rowExists(sqlite3* dbConnection, string sqlQuery) {
    sqlite3_stmt* sqlStatement = nullptr;
    bool exists = false;
    if (sqlite3_prepare_v2(dbConnection, sqlQuery.c_str(), -1, &sqlStatement, nullptr) == SQLITE_OK) {
        if (sqlite3_step(sqlStatement) == SQLITE_ROW) {
            exists = sqlite3_column_int(sqlStatement, 0) > 0;
        }
    }
    sqlite3_finalize(sqlStatement);
    return exists;
}


// ============================================================
//  executeNonQuery  —  helper para INSERT, UPDATE, DELETE
//  executeQuery<T>  —  helper para SELECT
// ============================================================

int Connection::executeNonQuery(string sqlQuery, vector<SqlParam> params) {
    sqlite3_stmt* sqlStatement = nullptr;

    if (sqlite3_prepare_v2(db_, sqlQuery.c_str(), -1, &sqlStatement, nullptr) != SQLITE_OK) {
        return SQLITE_ERROR;
    }

    if (!bindParams(sqlStatement, params)) {
        sqlite3_finalize(sqlStatement);
        return SQLITE_ERROR;
    }

    int resultCode = sqlite3_step(sqlStatement);
    sqlite3_finalize(sqlStatement);
    return resultCode;
}

template <typename T>
DBQueryResponse<T> Connection::executeQuery(string sqlQuery, vector<SqlParam> params, function<T(sqlite3_stmt*)> rowMapper, int successCode, string callerName)
{
    DBQueryResponse<T> queryResult;
    sqlite3_stmt* sqlStatement = nullptr;

    if (sqlite3_prepare_v2(db_, sqlQuery.c_str(), -1, &sqlStatement, nullptr) != SQLITE_OK) {
        DbResponse errorResponse = sqliteError(CODE_ERROR_DB, callerName + "::prepare");
        queryResult.code = errorResponse.code;
        queryResult.message = errorResponse.message;
        return queryResult;
    }

    if (!bindParams(sqlStatement, params)) {
        sqlite3_finalize(sqlStatement);
        DbResponse errorResponse = sqliteError(CODE_ERROR_DB, callerName + "::bind");
        queryResult.code = errorResponse.code;
        queryResult.message = errorResponse.message;
        return queryResult;
    }

    int lastStepResult = SQLITE_DONE;
    while ((lastStepResult = sqlite3_step(sqlStatement)) == SQLITE_ROW) {
        queryResult.data.push_back(rowMapper(sqlStatement));
    }

    sqlite3_finalize(sqlStatement);

    if (lastStepResult != SQLITE_DONE) {
        DbResponse errorResponse = sqliteError(CODE_ERROR_DB, callerName + "::step");
        queryResult.code = errorResponse.code;
        queryResult.message = errorResponse.message;
        return queryResult;
    }

    queryResult.code = successCode;
    return queryResult;
}


// ============================================================
//  Connection
// ============================================================

Connection::Connection() : db_(nullptr), opened_(false) {}

Connection::~Connection() {
    close();
}

DbResponse Connection::open(string dbPath) {
    try {
        if (opened_) {
            close();
        }

        int resultCode = sqlite3_open(dbPath.c_str(), &db_);
        if (resultCode != SQLITE_OK) {
            return sqliteError(CODE_ERROR_DB_CONNECTION, "open");
        }

        sqlite3_exec(db_, "PRAGMA FOREIGN_KEYS = ON;", nullptr, nullptr, nullptr);
        opened_ = true;
        return initTables();
    }
    catch (exception& e) {
        cout << "DB Exception: " << string(e.what());
        return { -1, CODE_EXCEPTION, "Excepcion no esperada al abrir la conexion" };
    }
    catch (...) {
        return { -1, CODE_EXCEPTION, "Excepcion desconocida al abrir la conexion" };
    }
}

void Connection::close() {
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
        opened_ = false;
    }
}

bool Connection::isOpen() {
    return opened_;
}

DbResponse Connection::sqliteError(int code, string context) {
    string errorMessage = "Error SQLite en [" + context + "]: ";
    errorMessage += db_ ? sqlite3_errmsg(db_) : "sin conexion";
    return { -1, code, errorMessage };
}

DbResponse Connection::initTables() {
    try {
        if (!execSimple(db_,
            "CREATE TABLE IF NOT EXISTS TB_TOURNAMENT ("
            "  ID INTEGER PRIMARY KEY AUTOINCREMENT,"
            "  NAME TEXT NOT NULL CHECK(NAME != ''),"
            "  PHASE TEXT NOT NULL DEFAULT 'Registro' CHECK(PHASE IN ('Registro', 'Grupos', 'Eliminacion', 'Finalizado'))"
            ");"))
            return sqliteError(CODE_ERROR_DB_INIT, "initTables::TB_TOURNAMENT");

        if (!execSimple(db_,
            "CREATE TABLE IF NOT EXISTS TB_TEAM ("
            "  ID INTEGER PRIMARY KEY AUTOINCREMENT,"
            "  TOURNAMENT_ID INTEGER NOT NULL REFERENCES TB_TOURNAMENT(ID) ON DELETE CASCADE,"
            "  NAME TEXT NOT NULL CHECK(NAME != ''),"
            "  TOURNAMENTS INTEGER NOT NULL DEFAULT 0 CHECK(TOURNAMENTS >= 0),"
            "  POINTS INTEGER NOT NULL DEFAULT 0 CHECK(POINTS >= 0),"
            "  WINS INTEGER NOT NULL DEFAULT 0 CHECK(WINS >= 0),"
            "  DRAWS INTEGER NOT NULL DEFAULT 0 CHECK(DRAWS >= 0),"
            "  LOSSES INTEGER NOT NULL DEFAULT 0 CHECK(LOSSES >= 0)"
            ");"))
            return sqliteError(CODE_ERROR_DB_INIT, "initTables::TB_TEAM");

        if (!execSimple(db_,
            "CREATE TABLE IF NOT EXISTS TB_PLAYER ("
            "  ID INTEGER PRIMARY KEY AUTOINCREMENT,"
            "  TEAM_ID INTEGER NOT NULL REFERENCES TB_TEAM(ID) ON DELETE CASCADE,"
            "  NAME TEXT NOT NULL CHECK(NAME != '')"
            ");"))
            return sqliteError(CODE_ERROR_DB_INIT, "initTables::TB_PLAYER");

        if (!execSimple(db_,
            "CREATE TABLE IF NOT EXISTS TB_MATCH ("
            "  ID INTEGER PRIMARY KEY AUTOINCREMENT,"
            "  TOURNAMENT_ID INTEGER NOT NULL REFERENCES TB_TOURNAMENT(ID) ON DELETE CASCADE,"
            "  TEAM_A_ID INTEGER NOT NULL REFERENCES TB_TEAM(ID),"
            "  TEAM_B_ID INTEGER NOT NULL REFERENCES TB_TEAM(ID),"
            "  PHASE TEXT NOT NULL CHECK(PHASE IN ('Grupos', 'Eliminacion')),"
            "  ROUND INTEGER NOT NULL DEFAULT 1 CHECK(ROUND >= 1),"
            "  STATUS TEXT NOT NULL DEFAULT 'Pendiente' CHECK(STATUS IN ('Pendiente', 'Finalizado')),"
            "  WINNER_ID INTEGER REFERENCES TB_TEAM(ID),"
            "  RESULT TEXT CHECK(RESULT IN ('Gana A', 'Gana B', 'Empate', NULL)),"
            "  QUEUE_POSITION INTEGER NOT NULL DEFAULT 0,"
            "  CHECK(TEAM_A_ID != TEAM_B_ID)"
            ");"))
            return sqliteError(CODE_ERROR_DB_INIT, "initTables::TB_MATCH");

        if (!execSimple(db_,
            "CREATE TABLE IF NOT EXISTS TB_PETITION ("
            "  ID INTEGER PRIMARY KEY AUTOINCREMENT,"
            "  REQUESTER_NAME TEXT NOT NULL CHECK(REQUESTER_NAME != ''),"
            "  TYPE TEXT NOT NULL CHECK(TYPE IN ('Inscripcion', 'Consulta', 'Apelacion')),"
            "  DESCRIPTION TEXT NOT NULL,"
            "  RESPONSE TEXT NOT NULL DEFAULT '',"
            "  STATUS TEXT NOT NULL DEFAULT 'Pendiente' CHECK(STATUS IN ('Pendiente', 'Atendida', 'Cancelada')),"
            "  QUEUE_POSITION INTEGER NOT NULL DEFAULT 0"
            ");"))
            return sqliteError(CODE_ERROR_DB_INIT, "initTables::TB_PETITION");

        if (!execSimple(db_,
            "CREATE TABLE IF NOT EXISTS TB_HISTORIC ("
            "  ID INTEGER PRIMARY KEY AUTOINCREMENT,"
            "  ACTION_TYPE TEXT NOT NULL,"
            "  ENTITY_TYPE TEXT NOT NULL,"
            "  RECORD_ID INTEGER,"
            "  PREVIOUS_DATA TEXT NOT NULL DEFAULT '{}',"
            "  NEW_DATA TEXT NOT NULL DEFAULT '{}',"
            "  STACK_POSITION INTEGER NOT NULL DEFAULT 0"
            ");"))
            return sqliteError(CODE_ERROR_DB_INIT, "initTables::TB_HISTORIC");

        execSimple(db_, "CREATE INDEX IF NOT EXISTS IDX_TEAM_TOURNAMENT ON TB_TEAM(TOURNAMENT_ID);");
        execSimple(db_, "CREATE INDEX IF NOT EXISTS IDX_PLAYER_TEAM ON TB_PLAYER(TEAM_ID);");
        execSimple(db_, "CREATE INDEX IF NOT EXISTS IDX_MATCH_TOURNAMENT ON TB_MATCH(TOURNAMENT_ID, QUEUE_POSITION);");
        execSimple(db_, "CREATE INDEX IF NOT EXISTS IDX_MATCH_STATUS ON TB_MATCH(TOURNAMENT_ID, PHASE, STATUS);");
        execSimple(db_, "CREATE INDEX IF NOT EXISTS IDX_PETITION_QUEUE ON TB_PETITION(STATUS, QUEUE_POSITION);");
        execSimple(db_, "CREATE INDEX IF NOT EXISTS IDX_HISTORIC_STACK ON TB_HISTORIC(TOURNAMENT_ID, STACK_POSITION DESC);");

        return { 1, CODE_SUCCESS, "Base de datos inicializada correctamente" };
    }
    catch (exception& e) {
        cout << "DB Exception: " << string(e.what());
        return { -1, CODE_EXCEPTION, "Excepcion no esperada en initTables" };
    }
    catch (...) {
        return { -1, CODE_EXCEPTION, "Excepcion desconocida en initTables" };
    }
}


// ============================================================
//  TB_TOURNAMENT
// ============================================================

DbResponse Connection::insertTournament(string name) {
    try {
        if (name.empty()) {
            return { -1, CODE_TOURNAMENT_INVALID_DATA, "El nombre del torneo no puede estar vacio" };
        }

        string checkDuplicateQuery = "SELECT COUNT(*) FROM TB_TOURNAMENT WHERE NAME = '" + name + "';";
        if (rowExists(db_, checkDuplicateQuery)) {
            return { -1, CODE_TOURNAMENT_DUPLICATE, "Ya existe un torneo con el nombre: " + name };
        }

        string sqlQuery = "INSERT INTO TB_TOURNAMENT (NAME) VALUES (?);";
        int resultCode = executeNonQuery(sqlQuery, { paramText(name) });

        if (resultCode != SQLITE_DONE) {
            return sqliteError(CODE_ERROR_DB, "insertTournament");
        }

        int insertedId = static_cast<int>(sqlite3_last_insert_rowid(db_));
        return { insertedId, CODE_TOURNAMENT_CREATED, "Torneo creado exitosamente" };
    }
    catch (exception& e) {
        cout << "DB Exception: " << string(e.what());
        return { -1, CODE_EXCEPTION, "Excepcion no esperada en insertTournament" };
    }
    catch (...) {
        return { -1, CODE_EXCEPTION, "Excepcion desconocida en insertTournament" };
    }
}

DBQueryResponse<Tournament> Connection::listTournaments() {
    try {
        string sqlQuery = "SELECT ID, NAME, PHASE FROM TB_TOURNAMENT ORDER BY ID;";

        auto rowMapper = [](sqlite3_stmt* sqlStatement) {
            Tournament tournamentRow;
            tournamentRow.id = sqlite3_column_int(sqlStatement, 0);
            tournamentRow.name = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 1));
            tournamentRow.phase = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 2));
            return tournamentRow;
            };

        DBQueryResponse<Tournament> queryResult = executeQuery<Tournament>(
            sqlQuery, {}, rowMapper, CODE_TOURNAMENT_LISTED, "listTournaments"
        );

        if (queryResult.code == CODE_TOURNAMENT_LISTED) {
            queryResult.message = queryResult.data.empty()
                ? "No hay torneos registrados"
                : "Torneos obtenidos: " + to_string(queryResult.data.size());
        }

        return queryResult;
    }
    catch (exception& e) {
        cout << "DB Exception: " << string(e.what());
        return { {}, CODE_EXCEPTION, "Excepcion no esperada en listTournaments" };
    }
    catch (...) {
        return { {}, CODE_EXCEPTION, "Excepcion desconocida en listTournaments" };
    }
}

DBQueryResponse<Tournament> Connection::obtainTournamentById(int id) {
    try {
        string sqlQuery = "SELECT ID, NAME, PHASE FROM TB_TOURNAMENT WHERE ID = ?;";

        auto rowMapper = [](sqlite3_stmt* sqlStatement) {
            Tournament tournamentRow;
            tournamentRow.id = sqlite3_column_int(sqlStatement, 0);
            tournamentRow.name = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 1));
            tournamentRow.phase = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 2));
            return tournamentRow;
            };

        DBQueryResponse<Tournament> queryResult = executeQuery<Tournament>(
            sqlQuery, { paramInt(id) }, rowMapper, CODE_TOURNAMENT_LISTED, "obtainTournamentById"
        );

        if (queryResult.code == CODE_TOURNAMENT_LISTED) {
            if (queryResult.data.empty()) {
                queryResult.code = CODE_TOURNAMENT_NOT_FOUND;
                queryResult.message = "Torneo con ID " + to_string(id) + " no encontrado";
            }
            else {
                queryResult.message = "Torneo encontrado";
            }
        }

        return queryResult;
    }
    catch (exception& e) {
        cout << "DB Exception: " << string(e.what());
        return { {}, CODE_EXCEPTION, "Excepcion no esperada en obtainTournamentById" };
    }
    catch (...) {
        return { {}, CODE_EXCEPTION, "Excepcion desconocida en obtainTournamentById" };
    }
}

DbResponse Connection::updateTournamentPhase(int id, string phase) {
    try {
        if (phase != "Registro" && phase != "Grupos" && phase != "Eliminacion" && phase != "Finalizado") {
            return { -1, CODE_TOURNAMENT_INVALID_DATA, "Fase invalida. Use: Registro, Grupos, Eliminacion o Finalizado" };
        }

        string checkExistQuery = "SELECT COUNT(*) FROM TB_TOURNAMENT WHERE ID = " + to_string(id) + ";";
        if (!rowExists(db_, checkExistQuery)) {
            return { -1, CODE_TOURNAMENT_NOT_FOUND, "Torneo con ID " + to_string(id) + " no encontrado" };
        }

        string sqlQuery = "UPDATE TB_TOURNAMENT SET PHASE = ? WHERE ID = ?;";
        int resultCode = executeNonQuery(sqlQuery, { paramText(phase), paramInt(id) });

        if (resultCode != SQLITE_DONE) {
            return sqliteError(CODE_ERROR_DB, "updateTournamentPhase");
        }

        return { id, CODE_TOURNAMENT_UPDATED, "Fase del torneo actualizada a: " + phase };
    }
    catch (exception& e) {
        cout << "DB Exception: " << string(e.what());
        return { -1, CODE_EXCEPTION, "Excepcion no esperada en updateTournamentPhase" };
    }
    catch (...) {
        return { -1, CODE_EXCEPTION, "Excepcion desconocida en updateTournamentPhase" };
    }
}

DbResponse Connection::updateTournamentName(int id, string newName) {
    try {
        if (newName.empty()) {
            return { -1, CODE_TOURNAMENT_INVALID_DATA, "El nombre del torneo no puede estar vacio" };
        }

        string checkExistQuery = "SELECT COUNT(*) FROM TB_TOURNAMENT WHERE ID = " + to_string(id) + ";";
        if (!rowExists(db_, checkExistQuery)) {
            return { -1, CODE_TOURNAMENT_NOT_FOUND, "Torneo con ID " + to_string(id) + " no encontrado" };
        }

        string sqlQuery = "UPDATE TB_TOURNAMENT SET NAME = ? WHERE ID = ?;";
        int resultCode = executeNonQuery(sqlQuery, { paramText(newName), paramInt(id) });

        if (resultCode != SQLITE_DONE) {
            return sqliteError(CODE_ERROR_DB, "updateTournamentName");
        }

        return { id, CODE_TOURNAMENT_UPDATED, "Nombre del torneo actualizado" };
    }
    catch (exception& e) {
        cout << "DB Exception: " << string(e.what());
        return { -1, CODE_EXCEPTION, "Excepcion no esperada en updateTournamentName" };
    }
    catch (...) {
        return { -1, CODE_EXCEPTION, "Excepcion desconocida en updateTournamentName" };
    }
}

DbResponse Connection::deleteTournament(int id) {
    try {
        string checkExistQuery = "SELECT COUNT(*) FROM TB_TOURNAMENT WHERE ID = " + to_string(id) + ";";
        if (!rowExists(db_, checkExistQuery)) {
            return { -1, CODE_TOURNAMENT_NOT_FOUND, "Torneo con ID " + to_string(id) + " no encontrado" };
        }

        string sqlQuery = "DELETE FROM TB_TOURNAMENT WHERE ID = ?;";
        int resultCode = executeNonQuery(sqlQuery, { paramInt(id) });

        if (resultCode != SQLITE_DONE) {
            return sqliteError(CODE_ERROR_DB, "deleteTournament");
        }

        return { id, CODE_TOURNAMENT_DELETED, "Torneo eliminado exitosamente" };
    }
    catch (exception& e) {
        cout << "DB Exception: " << string(e.what());
        return { -1, CODE_EXCEPTION, "Excepcion no esperada en deleteTournament" };
    }
    catch (...) {
        return { -1, CODE_EXCEPTION, "Excepcion desconocida en deleteTournament" };
    }
}


// ============================================================
//  TB_TEAM
// ============================================================

// Mapper reutilizable para filas de Team
static Team mapTeamRow(sqlite3_stmt* sqlStatement) {
    Team teamRow;
    teamRow.id = sqlite3_column_int(sqlStatement, 0);
    teamRow.tournamentId = sqlite3_column_int(sqlStatement, 1);
    teamRow.name = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 2));
    teamRow.tournaments = sqlite3_column_int(sqlStatement, 3);
    teamRow.points = sqlite3_column_int(sqlStatement, 4);
    teamRow.wins = sqlite3_column_int(sqlStatement, 5);
    teamRow.draws = sqlite3_column_int(sqlStatement, 6);
    teamRow.losses = sqlite3_column_int(sqlStatement, 7);
    return teamRow;
}

DbResponse Connection::insertTeam(string name, int tournamentId) {
    try {
        if (name.empty()) {
            return { -1, CODE_TEAM_INVALID_DATA, "El nombre del equipo no puede estar vacio" };
        }

        string checkTournamentQuery = "SELECT COUNT(*) FROM TB_TOURNAMENT WHERE ID = " + to_string(tournamentId) + ";";
        if (!rowExists(db_, checkTournamentQuery)) {
            return { -1, CODE_TOURNAMENT_NOT_FOUND, "El torneo con ID " + to_string(tournamentId) + " no existe" };
        }

        string checkDuplicateQuery = "SELECT COUNT(*) FROM TB_TEAM WHERE NAME = '" + name + "' AND TOURNAMENT_ID = " + to_string(tournamentId) + ";";
        if (rowExists(db_, checkDuplicateQuery)) {
            return { -1, CODE_TEAM_DUPLICATE, "Ya existe un equipo con ese nombre en el torneo" };
        }

        string sqlQuery = "INSERT INTO TB_TEAM (NAME, TOURNAMENT_ID) VALUES (?, ?);";
        int resultCode = executeNonQuery(sqlQuery, { paramText(name), paramInt(tournamentId) });

        if (resultCode != SQLITE_DONE) {
            return sqliteError(CODE_ERROR_DB, "insertTeam");
        }

        int insertedId = static_cast<int>(sqlite3_last_insert_rowid(db_));
        return { insertedId, CODE_TEAM_CREATED, "Equipo generado exitosamente" };
    }
    catch (exception& e) {
        cout << "DB Exception: " << string(e.what());
        return { -1, CODE_EXCEPTION, "Excepcion no esperada en insertTeam" };
    }
    catch (...) {
        return { -1, CODE_EXCEPTION, "Excepcion desconocida en insertTeam" };
    }
}

DBQueryResponse<Team> Connection::listAllTeams() {
    try {
        string sqlQuery =
            "SELECT ID, TOURNAMENT_ID, NAME, TOURNAMENTS, POINTS, WINS, DRAWS, LOSSES "
            "FROM TB_TEAM ORDER BY ID;";

        DBQueryResponse<Team> queryResult = executeQuery<Team>(
            sqlQuery, {}, mapTeamRow, CODE_TEAM_LISTED, "listAllTeams"
        );

        if (queryResult.code == CODE_TEAM_LISTED) {
            queryResult.message = queryResult.data.empty()
                ? "No hay equipos registrados"
                : "Equipos obtenidos: " + to_string(queryResult.data.size());
        }

        return queryResult;
    }
    catch (exception& e) {
        cout << "DB Exception: " << string(e.what());
        return { {}, CODE_EXCEPTION, "Excepcion no esperada en listAllTeams" };
    }
    catch (...) {
        return { {}, CODE_EXCEPTION, "Excepcion desconocida en listAllTeams" };
    }
}

DBQueryResponse<Team> Connection::listTeamsByTournament(int tournamentId) {
    try {
        string checkTournamentQuery = "SELECT COUNT(*) FROM TB_TOURNAMENT WHERE ID = " + to_string(tournamentId) + ";";
        if (!rowExists(db_, checkTournamentQuery)) {
            return { {}, CODE_TOURNAMENT_NOT_FOUND, "Torneo con ID " + to_string(tournamentId) + " no encontrado" };
        }

        string sqlQuery =
            "SELECT ID, TOURNAMENT_ID, NAME, TOURNAMENTS, POINTS, WINS, DRAWS, LOSSES "
            "FROM TB_TEAM WHERE TOURNAMENT_ID = ? ORDER BY ID;";

        DBQueryResponse<Team> queryResult = executeQuery<Team>(
            sqlQuery, { paramInt(tournamentId) }, mapTeamRow, CODE_TEAM_LISTED, "listTeamsByTournament"
        );

        if (queryResult.code == CODE_TEAM_LISTED) {
            queryResult.message = queryResult.data.empty()
                ? "No hay equipos en este torneo"
                : "Equipos obtenidos: " + to_string(queryResult.data.size());
        }

        return queryResult;
    }
    catch (exception& e) {
        cout << "DB Exception: " << string(e.what());
        return { {}, CODE_EXCEPTION, "Excepcion no esperada en listTeamsByTournament" };
    }
    catch (...) {
        return { {}, CODE_EXCEPTION, "Excepcion desconocida en listTeamsByTournament" };
    }
}

DBQueryResponse<Team> Connection::obtainTeamById(int id) {
    try {
        string sqlQuery =
            "SELECT ID, TOURNAMENT_ID, NAME, TOURNAMENTS, POINTS, WINS, DRAWS, LOSSES "
            "FROM TB_TEAM WHERE ID = ?;";

        DBQueryResponse<Team> queryResult = executeQuery<Team>(
            sqlQuery, { paramInt(id) }, mapTeamRow, CODE_TEAM_LISTED, "obtainTeamById"
        );

        if (queryResult.code == CODE_TEAM_LISTED) {
            if (queryResult.data.empty()) {
                queryResult.code = CODE_TEAM_NOT_FOUND;
                queryResult.message = "Equipo con ID " + to_string(id) + " no encontrado";
            }
            else {
                queryResult.message = "Equipo encontrado";
            }
        }

        return queryResult;
    }
    catch (exception& e) {
        cout << "DB Exception: " << string(e.what());
        return { {}, CODE_EXCEPTION, "Excepcion no esperada en obtainTeamById" };
    }
    catch (...) {
        return { {}, CODE_EXCEPTION, "Excepcion desconocida en obtainTeamById" };
    }
}

DbResponse Connection::updateTeam(int id, string newName) {
    try {
        if (newName.empty()) {
            return { -1, CODE_TEAM_INVALID_DATA, "El nombre del equipo no puede estar vacio" };
        }

        string checkExistQuery = "SELECT COUNT(*) FROM TB_TEAM WHERE ID = " + to_string(id) + ";";
        if (!rowExists(db_, checkExistQuery)) {
            return { -1, CODE_TEAM_NOT_FOUND, "Equipo con ID " + to_string(id) + " no encontrado" };
        }

        string sqlQuery = "UPDATE TB_TEAM SET NAME = ? WHERE ID = ?;";
        int resultCode = executeNonQuery(sqlQuery, { paramText(newName), paramInt(id) });

        if (resultCode != SQLITE_DONE) {
            return sqliteError(CODE_ERROR_DB, "updateTeam");
        }

        return { id, CODE_TEAM_UPDATED, "Equipo actualizado exitosamente" };
    }
    catch (exception& e) {
        cout << "DB Exception: " << string(e.what());
        return { -1, CODE_EXCEPTION, "Excepcion no esperada en updateTeam" };
    }
    catch (...) {
        return { -1, CODE_EXCEPTION, "Excepcion desconocida en updateTeam" };
    }
}

DbResponse Connection::updateTeamStats(int id, int points, int wins, int losses, int draws) {
    try {
        string checkExistQuery = "SELECT COUNT(*) FROM TB_TEAM WHERE ID = " + to_string(id) + ";";
        if (!rowExists(db_, checkExistQuery)) {
            return { -1, CODE_TEAM_NOT_FOUND, "Equipo con ID " + to_string(id) + " no encontrado" };
        }

        string sqlQuery = "UPDATE TB_TEAM SET POINTS = POINTS + ?, WINS = WINS + ?, DRAWS = DRAWS + ?, LOSSES = LOSSES + ? WHERE ID = ? ;";
        int resultCode = executeNonQuery(sqlQuery, {
            paramInt(points), paramInt(wins), paramInt(draws), paramInt(losses), paramInt(id)
            });

        if (resultCode != SQLITE_DONE) {
            return sqliteError(CODE_ERROR_DB, "updateTeamStats");
        }

        return { id, CODE_TEAM_UPDATED, "Estadisticas del equipo actualizadas" };
    }
    catch (exception& e) {
        cout << "DB Exception: " << string(e.what());
        return { -1, CODE_EXCEPTION, "Excepcion no esperada en updateTeamStats" };
    }
    catch (...) {
        return { -1, CODE_EXCEPTION, "Excepcion desconocida en updateTeamStats" };
    }
}

DbResponse Connection::deleteTeam(int id) {
    try {
        string checkExistQuery = "SELECT COUNT(*) FROM TB_TEAM WHERE ID = " + to_string(id) + ";";
        if (!rowExists(db_, checkExistQuery)) {
            return { -1, CODE_TEAM_NOT_FOUND, "Equipo con ID " + to_string(id) + " no encontrado" };
        }

        string sqlQuery = "DELETE FROM TB_TEAM WHERE ID = ?;";
        int resultCode = executeNonQuery(sqlQuery, { paramInt(id) });

        if (resultCode != SQLITE_DONE) {
            return sqliteError(CODE_ERROR_DB, "deleteTeam");
        }

        return { id, CODE_TEAM_DELETED, "Equipo eliminado exitosamente" };
    }
    catch (exception& e) {
        cout << "DB Exception: " << string(e.what());
        return { -1, CODE_EXCEPTION, "Excepcion no esperada en deleteTeam" };
    }
    catch (...) {
        return { -1, CODE_EXCEPTION, "Excepcion desconocida en deleteTeam" };
    }
}


// ============================================================
//  TB_PLAYER
// ============================================================

static Player mapPlayerRow(sqlite3_stmt* sqlStatement) {
    Player playerRow;
    playerRow.id = sqlite3_column_int(sqlStatement, 0);
    playerRow.teamId = sqlite3_column_int(sqlStatement, 1);
    playerRow.name = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 2));
    return playerRow;
}

DbResponse Connection::insertPlayer(string name, int teamId) {
    try {
        if (name.empty()) {
            return { -1, CODE_PLAYER_INVALID_DATA, "El nombre del jugador no puede estar vacio" };
        }

        string checkTeamQuery = "SELECT COUNT(*) FROM TB_TEAM WHERE ID = " + to_string(teamId) + ";";
        if (!rowExists(db_, checkTeamQuery)) {
            return { -1, CODE_TEAM_NOT_FOUND, "El equipo con ID " + to_string(teamId) + " no existe" };
        }

        string checkDuplicateQuery = "SELECT COUNT(*) FROM TB_PLAYER WHERE NAME = '" + name + "' AND TEAM_ID = " + to_string(teamId) + ";";
        if (rowExists(db_, checkDuplicateQuery)) {
            return { -1, CODE_PLAYER_DUPLICATE, "Ya existe un jugador con ese nombre en el equipo" };
        }

        string sqlQuery = "INSERT INTO TB_PLAYER (NAME, TEAM_ID) VALUES (?, ?);";
        int resultCode = executeNonQuery(sqlQuery, { paramText(name), paramInt(teamId) });

        if (resultCode != SQLITE_DONE) {
            return sqliteError(CODE_ERROR_DB, "insertPlayer");
        }

        int insertedId = static_cast<int>(sqlite3_last_insert_rowid(db_));
        return { insertedId, CODE_PLAYER_CREATED, "Jugador registrado exitosamente" };
    }
    catch (exception& e) {
        cout << "DB Exception: " << string(e.what());
        return { -1, CODE_EXCEPTION, "Excepcion no esperada en insertPlayer" };
    }
    catch (...) {
        return { -1, CODE_EXCEPTION, "Excepcion desconocida en insertPlayer" };
    }
}

DBQueryResponse<Player> Connection::listAllPlayers() {
    try {
        string sqlQuery = "SELECT ID, TEAM_ID, NAME FROM TB_PLAYER ORDER BY ID;";

        DBQueryResponse<Player> queryResult = executeQuery<Player>(
            sqlQuery, {}, mapPlayerRow, CODE_PLAYER_LISTED, "listAllPlayers"
        );

        if (queryResult.code == CODE_PLAYER_LISTED) {
            queryResult.message = queryResult.data.empty()
                ? "No hay jugadores registrados"
                : "Jugadores obtenidos: " + to_string(queryResult.data.size());
        }

        return queryResult;
    }
    catch (exception& e) {
        cout << "DB Exception: " << string(e.what());
        return { {}, CODE_EXCEPTION, "Excepcion no esperada en listAllPlayers" };
    }
    catch (...) {
        return { {}, CODE_EXCEPTION, "Excepcion desconocida en listAllPlayers" };
    }
}

DBQueryResponse<Player> Connection::listPlayersByTeam(int teamId) {
    try {
        string checkTeamQuery = "SELECT COUNT(*) FROM TB_TEAM WHERE ID = " + to_string(teamId) + ";";
        if (!rowExists(db_, checkTeamQuery)) {
            return { {}, CODE_TEAM_NOT_FOUND, "Equipo con ID " + to_string(teamId) + " no encontrado" };
        }

        string sqlQuery = "SELECT ID, TEAM_ID, NAME FROM TB_PLAYER WHERE TEAM_ID = ? ORDER BY ID;";

        DBQueryResponse<Player> queryResult = executeQuery<Player>(
            sqlQuery, { paramInt(teamId) }, mapPlayerRow, CODE_PLAYER_LISTED, "listPlayersByTeam"
        );

        if (queryResult.code == CODE_PLAYER_LISTED) {
            queryResult.message = queryResult.data.empty()
                ? "No hay jugadores en este equipo"
                : "Jugadores obtenidos: " + to_string(queryResult.data.size());
        }

        return queryResult;
    }
    catch (exception& e) {
        cout << "DB Exception: " << string(e.what());
        return { {}, CODE_EXCEPTION, "Excepcion no esperada en listPlayersByTeam" };
    }
    catch (...) {
        return { {}, CODE_EXCEPTION, "Excepcion desconocida en listPlayersByTeam" };
    }
}

DBQueryResponse<Player> Connection::obtainPlayerById(int id) {
    try {
        string sqlQuery = "SELECT ID, TEAM_ID, NAME FROM TB_PLAYER WHERE ID = ?;";

        DBQueryResponse<Player> queryResult = executeQuery<Player>(
            sqlQuery, { paramInt(id) }, mapPlayerRow, CODE_PLAYER_LISTED, "obtainPlayerById"
        );

        if (queryResult.code == CODE_PLAYER_LISTED) {
            if (queryResult.data.empty()) {
                queryResult.code = CODE_PLAYER_NOT_FOUND;
                queryResult.message = "Jugador con ID " + to_string(id) + " no encontrado";
            }
            else {
                queryResult.message = "Jugador encontrado";
            }
        }

        return queryResult;
    }
    catch (exception& e) {
        cout << "DB Exception: " << string(e.what());
        return { {}, CODE_EXCEPTION, "Excepcion no esperada en obtainPlayerById" };
    }
    catch (...) {
        return { {}, CODE_EXCEPTION, "Excepcion desconocida en obtainPlayerById" };
    }
}

DbResponse Connection::updatePlayer(int id, string name) {
    try {
        if (name.empty()) {
            return { -1, CODE_PLAYER_INVALID_DATA, "El nombre del jugador no puede estar vacio" };
        }

        string checkExistQuery = "SELECT COUNT(*) FROM TB_PLAYER WHERE ID = " + to_string(id) + ";";
        if (!rowExists(db_, checkExistQuery)) {
            return { -1, CODE_PLAYER_NOT_FOUND, "Jugador con ID " + to_string(id) + " no encontrado" };
        }

        string sqlQuery = "UPDATE TB_PLAYER SET NAME = ? WHERE ID = ?;";
        int resultCode = executeNonQuery(sqlQuery, { paramText(name), paramInt(id) });

        if (resultCode != SQLITE_DONE) {
            return sqliteError(CODE_ERROR_DB, "updatePlayer");
        }

        return { id, CODE_PLAYER_UPDATED, "Jugador actualizado exitosamente" };
    }
    catch (exception& e) {
        cout << "DB Exception: " << string(e.what());
        return { -1, CODE_EXCEPTION, "Excepcion no esperada en updatePlayer" };
    }
    catch (...) {
        return { -1, CODE_EXCEPTION, "Excepcion desconocida en updatePlayer" };
    }
}

DbResponse Connection::updatePlayerTeam(int playerId, int teamId) {
    try {
        string checkPlayerQuery = "SELECT COUNT(*) FROM TB_PLAYER WHERE ID = " + to_string(playerId) + ";";
        if (!rowExists(db_, checkPlayerQuery)) {
            return { -1, CODE_PLAYER_NOT_FOUND, "Jugador con ID " + to_string(playerId) + " no encontrado" };
        }

        string checkTeamQuery = "SELECT COUNT(*) FROM TB_TEAM WHERE ID = " + to_string(teamId) + ";";
        if (!rowExists(db_, checkTeamQuery)) {
            return { -1, CODE_TEAM_NOT_FOUND, "El equipo con ID " + to_string(teamId) + " no existe" };
        }

        string sqlQuery = "UPDATE TB_PLAYER SET TEAM_ID = ? WHERE ID = ?;";
        int resultCode = executeNonQuery(sqlQuery, { paramInt(teamId), paramInt(playerId) });

        if (resultCode != SQLITE_DONE) {
            return sqliteError(CODE_ERROR_DB, "updatePlayerTeam");
        }

        return { playerId, CODE_PLAYER_UPDATED, "Equipo del jugador actualizado exitosamente" };
    }
    catch (exception& e) {
        cout << "DB Exception: " << string(e.what());
        return { -1, CODE_EXCEPTION, "Excepcion no esperada en updatePlayerTeam" };
    }
    catch (...) {
        return { -1, CODE_EXCEPTION, "Excepcion desconocida en updatePlayerTeam" };
    }
}

DbResponse Connection::deletePlayer(int id) {
    try {
        string checkExistQuery = "SELECT COUNT(*) FROM TB_PLAYER WHERE ID = " + to_string(id) + ";";
        if (!rowExists(db_, checkExistQuery)) {
            return { -1, CODE_PLAYER_NOT_FOUND, "Jugador con ID " + to_string(id) + " no encontrado" };
        }

        string sqlQuery = "DELETE FROM TB_PLAYER WHERE ID = ?;";
        int resultCode = executeNonQuery(sqlQuery, { paramInt(id) });

        if (resultCode != SQLITE_DONE) {
            return sqliteError(CODE_ERROR_DB, "deletePlayer");
        }

        return { id, CODE_PLAYER_DELETED, "Jugador eliminado exitosamente" };
    }
    catch (exception& e) {
        cout << "DB Exception: " << string(e.what());
        return { -1, CODE_EXCEPTION, "Excepcion no esperada en deletePlayer" };
    }
    catch (...) {
        return { -1, CODE_EXCEPTION, "Excepcion desconocida en deletePlayer" };
    }
}


// ============================================================
//  TB_MATCH
// ============================================================

static Match mapMatchRow(sqlite3_stmt* sqlStatement) {
    Match matchRow;
    matchRow.id = sqlite3_column_int(sqlStatement, 0);
    matchRow.tournamentId = sqlite3_column_int(sqlStatement, 1);
    matchRow.teamAId = sqlite3_column_int(sqlStatement, 2);
    matchRow.teamBId = sqlite3_column_int(sqlStatement, 3);
    matchRow.phase = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 4));
    matchRow.round = sqlite3_column_int(sqlStatement, 5);
    matchRow.status = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 6));
    matchRow.winnerId = sqlite3_column_int(sqlStatement, 7);

    const unsigned char* resultText = sqlite3_column_text(sqlStatement, 8);
    const unsigned char* playedAtText = sqlite3_column_text(sqlStatement, 9);
    matchRow.result = resultText ? reinterpret_cast<const char*>(resultText) : "";
    matchRow.queuePosition = sqlite3_column_int(sqlStatement, 10);
    return matchRow;
}

DbResponse Connection::insertMatch(int tournamentId, int teamAId, int teamBId, string phase, int round, int queuePosition) {
    try {
        if (phase != "Grupos" && phase != "Eliminacion") {
            return { -1, CODE_MATCH_INVALID_DATA, "Fase invalida. Use: Grupos o Eliminacion" };
        }

        if (teamAId == teamBId) {
            return { -1, CODE_MATCH_INVALID_DATA, "Un equipo no puede jugar contra si mismo" };
        }

        string checkTournamentQuery = "SELECT COUNT(*) FROM TB_TOURNAMENT WHERE ID = " + to_string(tournamentId) + ";";
        if (!rowExists(db_, checkTournamentQuery)) {
            return { -1, CODE_TOURNAMENT_NOT_FOUND, "El torneo con ID " + to_string(tournamentId) + " no existe" };
        }

        string checkTeamAQuery = "SELECT COUNT(*) FROM TB_TEAM WHERE ID = " + to_string(teamAId) + ";";
        if (!rowExists(db_, checkTeamAQuery)) {
            return { -1, CODE_TEAM_NOT_FOUND, "El equipo A con ID " + to_string(teamAId) + " no existe" };
        }

        string checkTeamBQuery = "SELECT COUNT(*) FROM TB_TEAM WHERE ID = " + to_string(teamBId) + ";";
        if (!rowExists(db_, checkTeamBQuery)) {
            return { -1, CODE_TEAM_NOT_FOUND, "El equipo B con ID " + to_string(teamBId) + " no existe" };
        }

        string sqlQuery =
            "INSERT INTO TB_MATCH (TOURNAMENT_ID, TEAM_A_ID, TEAM_B_ID, PHASE, ROUND, QUEUE_POSITION) "
            "VALUES (?, ?, ?, ?, ?, ?);";

        int resultCode = executeNonQuery(sqlQuery, {
            paramInt(tournamentId), paramInt(teamAId), paramInt(teamBId),
            paramText(phase), paramInt(round), paramInt(queuePosition)
            });

        if (resultCode != SQLITE_DONE) {
            return sqliteError(CODE_ERROR_DB, "insertMatch");
        }

        int insertedId = static_cast<int>(sqlite3_last_insert_rowid(db_));
        return { insertedId, CODE_MATCH_CREATED, "Partido creado exitosamente" };
    }
    catch (exception& e) {
        cout << "DB Exception: " << string(e.what());
        return { -1, CODE_EXCEPTION, "Excepcion no esperada en insertMatch" };
    }
    catch (...) {
        return { -1, CODE_EXCEPTION, "Excepcion desconocida en insertMatch" };
    }
}

DBQueryResponse<Match> Connection::listMatchesByTournament(int tournamentId) {
    try {
        string checkTournamentQuery = "SELECT COUNT(*) FROM TB_TOURNAMENT WHERE ID = " + to_string(tournamentId) + ";";
        if (!rowExists(db_, checkTournamentQuery)) {
            return { {}, CODE_TOURNAMENT_NOT_FOUND, "Torneo con ID " + to_string(tournamentId) + " no encontrado" };
        }

        string sqlQuery =
            "SELECT ID, TOURNAMENT_ID, TEAM_A_ID, TEAM_B_ID, PHASE, ROUND, STATUS, WINNER_ID, RESULT, PLAYED_AT, QUEUE_POSITION "
            "FROM TB_MATCH WHERE TOURNAMENT_ID = ? ORDER BY QUEUE_POSITION ASC;";

        DBQueryResponse<Match> queryResult = executeQuery<Match>(
            sqlQuery, { paramInt(tournamentId) }, mapMatchRow, CODE_MATCH_LISTED, "listMatchesByTournament"
        );

        if (queryResult.code == CODE_MATCH_LISTED) {
            queryResult.message = queryResult.data.empty()
                ? "No hay partidos en este torneo"
                : "Partidos obtenidos: " + to_string(queryResult.data.size());
        }

        return queryResult;
    }
    catch (exception& e) {
        cout << "DB Exception: " << string(e.what());
        return { {}, CODE_EXCEPTION, "Excepcion no esperada en listMatchesByTournament" };
    }
    catch (...) {
        return { {}, CODE_EXCEPTION, "Excepcion desconocida en listMatchesByTournament" };
    }
}

DBQueryResponse<Match> Connection::listMatchesByPhase(int tournamentId, string phase) {
    try {
        string sqlQuery =
            "SELECT ID, TOURNAMENT_ID, TEAM_A_ID, TEAM_B_ID, PHASE, ROUND, STATUS, WINNER_ID, RESULT, PLAYED_AT, QUEUE_POSITION "
            "FROM TB_MATCH WHERE TOURNAMENT_ID = ? AND PHASE = ? ORDER BY QUEUE_POSITION ASC;";

        DBQueryResponse<Match> queryResult = executeQuery<Match>(
            sqlQuery, { paramInt(tournamentId), paramText(phase) }, mapMatchRow, CODE_MATCH_LISTED, "listMatchesByPhase"
        );

        if (queryResult.code == CODE_MATCH_LISTED) {
            queryResult.message = queryResult.data.empty()
                ? "No hay partidos en la fase: " + phase
                : "Partidos obtenidos: " + to_string(queryResult.data.size());
        }

        return queryResult;
    }
    catch (exception& e) {
        cout << "DB Exception: " << string(e.what());
        return { {}, CODE_EXCEPTION, "Excepcion no esperada en listMatchesByPhase" };
    }
    catch (...) {
        return { {}, CODE_EXCEPTION, "Excepcion desconocida en listMatchesByPhase" };
    }
}

DBQueryResponse<Match> Connection::obtainMatchById(int id) {
    try {
        string sqlQuery =
            "SELECT ID, TOURNAMENT_ID, TEAM_A_ID, TEAM_B_ID, PHASE, ROUND, STATUS, WINNER_ID, RESULT, PLAYED_AT, QUEUE_POSITION "
            "FROM TB_MATCH WHERE ID = ?;";

        DBQueryResponse<Match> queryResult = executeQuery<Match>(
            sqlQuery, { paramInt(id) }, mapMatchRow, CODE_MATCH_LISTED, "obtainMatchById"
        );

        if (queryResult.code == CODE_MATCH_LISTED) {
            if (queryResult.data.empty()) {
                queryResult.code = CODE_MATCH_NOT_FOUND;
                queryResult.message = "Partido con ID " + to_string(id) + " no encontrado";
            }
            else {
                queryResult.message = "Partido encontrado";
            }
        }

        return queryResult;
    }
    catch (exception& e) {
        cout << "DB Exception: " << string(e.what());
        return { {}, CODE_EXCEPTION, "Excepcion no esperada en obtainMatchById" };
    }
    catch (...) {
        return { {}, CODE_EXCEPTION, "Excepcion desconocida en obtainMatchById" };
    }
}

DbResponse Connection::updateMatch(int id, string phase, int round, string status, int winnerId, string result) {
    try {
        string checkExistQuery = "SELECT COUNT(*) FROM TB_MATCH WHERE ID = " + to_string(id) + ";";
        if (!rowExists(db_, checkExistQuery)) {
            return { -1, CODE_MATCH_NOT_FOUND, "Partido con ID " + to_string(id) + " no encontrado" };
        }

        string sqlQuery =
            "UPDATE TB_MATCH SET PHASE = ?, ROUND = ?, STATUS = ?, WINNER_ID = ?, RESULT = ?, "
            "PLAYED_AT = datetime('now') WHERE ID = ?;";

        int resultCode = executeNonQuery(sqlQuery, {
            paramText(phase), paramInt(round), paramText(status),
            paramInt(winnerId), paramText(result), paramInt(id)
            });

        if (resultCode != SQLITE_DONE) {
            return sqliteError(CODE_ERROR_DB, "updateMatch");
        }

        return { id, CODE_MATCH_UPDATED, "Partido actualizado exitosamente" };
    }
    catch (exception& e) {
        cout << "DB Exception: " << string(e.what());
        return { -1, CODE_EXCEPTION, "Excepcion no esperada en updateMatch" };
    }
    catch (...) {
        return { -1, CODE_EXCEPTION, "Excepcion desconocida en updateMatch" };
    }
}

DbResponse Connection::deleteMatch(int id) {
    try {
        string checkExistQuery = "SELECT COUNT(*) FROM TB_MATCH WHERE ID = " + to_string(id) + ";";
        if (!rowExists(db_, checkExistQuery)) {
            return { -1, CODE_MATCH_NOT_FOUND, "Partido con ID " + to_string(id) + " no encontrado" };
        }

        string sqlQuery = "DELETE FROM TB_MATCH WHERE ID = ?;";
        int resultCode = executeNonQuery(sqlQuery, { paramInt(id) });

        if (resultCode != SQLITE_DONE) {
            return sqliteError(CODE_ERROR_DB, "deleteMatch");
        }

        return { id, CODE_MATCH_DELETED, "Partido eliminado exitosamente" };
    }
    catch (exception& e) {
        cout << "DB Exception: " << string(e.what());
        return { -1, CODE_EXCEPTION, "Excepcion no esperada en deleteMatch" };
    }
    catch (...) {
        return { -1, CODE_EXCEPTION, "Excepcion desconocida en deleteMatch" };
    }
}


// ============================================================
//  TB_PETITION
// ============================================================

static Petition mapPetitionRow(sqlite3_stmt* sqlStatement) {
    Petition petitionRow;
    petitionRow.id = sqlite3_column_int(sqlStatement, 0);
    petitionRow.requesterName = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 1));
    petitionRow.type = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 2));
    petitionRow.description = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 3));
    petitionRow.status = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 4));
    petitionRow.queuePosition = sqlite3_column_int(sqlStatement, 5);
    return petitionRow;
}

DbResponse Connection::insertPetition(string requesterName, string type, string description) {
    try {
        if (requesterName.empty()) {
            return { -1, CODE_PETITION_INVALID_DATA, "El nombre del solicitante no puede estar vacio" };
        }

        if (type != "Inscripcion" && type != "Consulta" && type != "Apelacion") {
            return { -1, CODE_PETITION_INVALID_DATA, "Tipo invalido. Use: Inscripcion, Consulta o Apelacion" };
        }

        // Calcular la siguiente posicion en la cola (FIFO: maximo actual + 1)
        string maxPositionQuery = "SELECT COALESCE(MAX(QUEUE_POSITION), 0) + 1 FROM TB_PETITION WHERE STATUS = 'Pendiente';";
        sqlite3_stmt* positionStatement = nullptr;
        int nextQueuePosition = 1;

        if (sqlite3_prepare_v2(db_, maxPositionQuery.c_str(), -1, &positionStatement, nullptr) == SQLITE_OK) {
            if (sqlite3_step(positionStatement) == SQLITE_ROW) {
                nextQueuePosition = sqlite3_column_int(positionStatement, 0);
            }
        }
        sqlite3_finalize(positionStatement);

        string sqlQuery =
            "INSERT INTO TB_PETITION (REQUESTER_NAME, TYPE, DESCRIPTION, QUEUE_POSITION) "
            "VALUES (?, ?, ?, ?);";

        int resultCode = executeNonQuery(sqlQuery, {
            paramText(requesterName), paramText(type), paramText(description), paramInt(nextQueuePosition)
            });

        if (resultCode != SQLITE_DONE) {
            return sqliteError(CODE_ERROR_DB, "insertPetition");
        }

        int insertedId = static_cast<int>(sqlite3_last_insert_rowid(db_));
        return { insertedId, CODE_PETITION_CREATED, "Peticion registrada en la cola" };
    }
    catch (exception& e) {
        cout << "DB Exception: " << string(e.what());
        return { -1, CODE_EXCEPTION, "Excepcion no esperada en insertPetition" };
    }
    catch (...) {
        return { -1, CODE_EXCEPTION, "Excepcion desconocida en insertPeticion" };
    }
}

DBQueryResponse<Petition> Connection::listPendingPetitions() {
    try {
        string sqlQuery =
            "SELECT ID, REQUESTER_NAME, TYPE, DESCRIPTION, STATUS, QUEUE_POSITION "
            "FROM TB_PETITION WHERE STATUS = 'Pendiente' ORDER BY QUEUE_POSITION ASC;";

        DBQueryResponse<Petition> queryResult = executeQuery<Petition>(
            sqlQuery, {}, mapPetitionRow, CODE_PETITION_LISTED, "listPendingPetitions"
        );

        if (queryResult.code == CODE_PETITION_LISTED) {
            queryResult.message = queryResult.data.empty()
                ? "No hay peticiones pendientes"
                : "Peticiones pendientes: " + to_string(queryResult.data.size());
        }

        return queryResult;
    }
    catch (exception& e) {
        cout << "DB Exception: " << string(e.what());
        return { {}, CODE_EXCEPTION, "Excepcion no esperada en listPendingPetitions" };
    }
    catch (...) {
        return { {}, CODE_EXCEPTION, "Excepcion desconocida en listPendingPetitions" };
    }
}

DBQueryResponse<Petition> Connection::obtainNextPetition() {
    try {
        string sqlQuery =
            "SELECT ID, REQUESTER_NAME, TYPE, DESCRIPTION, STATUS, QUEUE_POSITION "
            "FROM TB_PETITION WHERE STATUS = 'Pendiente' ORDER BY QUEUE_POSITION ASC LIMIT 1;";

        DBQueryResponse<Petition> queryResult = executeQuery<Petition>(
            sqlQuery, {}, mapPetitionRow, CODE_PETITION_LISTED, "obtainNextPetition"
        );

        if (queryResult.code == CODE_PETITION_LISTED) {
            if (queryResult.data.empty()) {
                queryResult.code = CODE_PETITION_NOT_FOUND;
                queryResult.message = "No hay peticiones pendientes en la cola";
            }
            else {
                queryResult.message = "Siguiente peticion obtenida";
            }
        }

        return queryResult;
    }
    catch (exception& e) {
        cout << "DB Exception: " << string(e.what());
        return { {}, CODE_EXCEPTION, "Excepcion no esperada en obtainNextPetition" };
    }
    catch (...) {
        return { {}, CODE_EXCEPTION, "Excepcion desconocida en obtainNextPetition" };
    }
}

DBQueryResponse<Petition> Connection::obtainPetitionById(int id) {
    try {
        string sqlQuery =
            "SELECT ID, REQUESTER_NAME, TYPE, DESCRIPTION, STATUS, QUEUE_POSITION "
            "FROM TB_PETITION WHERE ID = ?;";

        DBQueryResponse<Petition> queryResult = executeQuery<Petition>(
            sqlQuery, { paramInt(id) }, mapPetitionRow, CODE_PETITION_LISTED, "obtainPetitionById"
        );

        if (queryResult.code == CODE_PETITION_LISTED) {
            if (queryResult.data.empty()) {
                queryResult.code = CODE_PETITION_NOT_FOUND;
                queryResult.message = "Peticion con ID " + to_string(id) + " no encontrada";
            }
            else {
                queryResult.message = "Peticion encontrada";
            }
        }

        return queryResult;
    }
    catch (exception& e) {
        cout << "DB Exception: " << string(e.what());
        return { {}, CODE_EXCEPTION, "Excepcion no esperada en obtainPetitionById" };
    }
    catch (...) {
        return { {}, CODE_EXCEPTION, "Excepcion desconocida en obtainPetitionById" };
    }
}

DbResponse Connection::updatePetitionStatus(int id, string response, string status) {
    try {
        if (status != "Pendiente" && status != "Atendida" && status != "Cancelada") {
            return { -1, CODE_PETITION_INVALID_DATA, "Estado invalido. Use: Pendiente, Atendida o Cancelada" };
        }

        string checkExistQuery = "SELECT COUNT(*) FROM TB_PETITION WHERE ID = " + to_string(id) + ";";
        if (!rowExists(db_, checkExistQuery)) {
            return { -1, CODE_PETITION_NOT_FOUND, "Peticion con ID " + to_string(id) + " no encontrada" };
        }

        string sqlQuery = "UPDATE TB_PETITION SET RESPONSE = ?, STATUS = ? WHERE ID = ?;";
        int resultCode = executeNonQuery(sqlQuery, { paramText(response), paramText(status), paramInt(id) });

        if (resultCode != SQLITE_DONE) {
            return sqliteError(CODE_ERROR_DB, "updatePetitionStatus");
        }

        return { id, CODE_PETITION_ATTENDED, "Peticion actualizada exitosamente" };
    }
    catch (exception& e) {
        cout << "DB Exception: " << string(e.what());
        return { -1, CODE_EXCEPTION, "Excepcion no esperada en updatePetitionStatus" };
    }
    catch (...) {
        return { -1, CODE_EXCEPTION, "Excepcion desconocida en updatePetitionStatus" };
    }
}

DbResponse Connection::deletePetition(int id) {
    try {
        string checkExistQuery = "SELECT COUNT(*) FROM TB_PETITION WHERE ID = " + to_string(id) + ";";
        if (!rowExists(db_, checkExistQuery)) {
            return { -1, CODE_PETITION_NOT_FOUND, "Peticion con ID " + to_string(id) + " no encontrada" };
        }

        string sqlQuery = "DELETE FROM TB_PETITION WHERE ID = ?;";
        int resultCode = executeNonQuery(sqlQuery, { paramInt(id) });

        if (resultCode != SQLITE_DONE) {
            return sqliteError(CODE_ERROR_DB, "deletePetition");
        }

        return { id, CODE_PETITION_DELETED, "Peticion eliminada exitosamente" };
    }
    catch (exception& e) {
        cout << "DB Exception: " << string(e.what());
        return { -1, CODE_EXCEPTION, "Excepcion no esperada en deletePetition" };
    }
    catch (...) {
        return { -1, CODE_EXCEPTION, "Excepcion desconocida en deletePetition" };
    }
}


// ============================================================
//  TB_HISTORIC
// ============================================================

static Historic mapHistoricRow(sqlite3_stmt* sqlStatement) {
    Historic historicRow;
    historicRow.id = sqlite3_column_int(sqlStatement, 0);
    historicRow.actionType = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 1));
    historicRow.entityType = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 2));
    historicRow.recordId = sqlite3_column_int(sqlStatement, 3);
    historicRow.previousData = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 4));
    historicRow.newData = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 5));
    historicRow.stackPosition = sqlite3_column_int(sqlStatement, 6);
    return historicRow;
}

DbResponse Connection::insertHistoric(string actionType, string entityType, int recordId, string previousData, string newData, int stackPosition) {
    try {
        if (actionType.empty()) {
            return { -1, CODE_HISTORIC_INVALID_DATA, "El tipo de accion no puede estar vacio" };
        }

        if (entityType.empty()) {
            return { -1, CODE_HISTORIC_INVALID_DATA, "El tipo de entidad no puede estar vacio" };
        }

        if (previousData.empty()) {
            return { -1, CODE_HISTORIC_INVALID_DATA, "Los datos previos son necesarios para permitir deshacer" };
        }

        string sqlQuery =
            "INSERT INTO TB_HISTORIC (ACTION_TYPE, ENTITY_TYPE, RECORD_ID, PREVIOUS_DATA, NEW_DATA, STACK_POSITION) "
            "VALUES (?, ?, ?, ?, ?, ?);";

        int resultCode = executeNonQuery(sqlQuery, {
            paramText(actionType), paramText(entityType), paramInt(recordId),
            paramText(previousData), paramText(newData), paramInt(stackPosition)
            });

        if (resultCode != SQLITE_DONE) {
            return sqliteError(CODE_ERROR_DB, "insertHistoric");
        }

        int insertedId = static_cast<int>(sqlite3_last_insert_rowid(db_));
        return { insertedId, CODE_SUCCESS, "Accion registrada en el historial" };
    }
    catch (exception& e) {
        cout << "DB Exception: " << string(e.what());
        return { -1, CODE_EXCEPTION, "Excepcion no esperada en insertHistoric" };
    }
    catch (...) {
        return { -1, CODE_EXCEPTION, "Excepcion desconocida en insertHistoric" };
    }
}

DBQueryResponse<Historic> Connection::listHistoric() {
    try {
        string sqlQuery =
            "SELECT ID, ACTION_TYPE, ENTITY_TYPE, RECORD_ID, PREVIOUS_DATA, NEW_DATA, STACK_POSITION "
            "FROM TB_HISTORIC ORDER BY STACK_POSITION ASC;";

        DBQueryResponse<Historic> queryResult = executeQuery<Historic>(
            sqlQuery, {}, mapHistoricRow, CODE_HISTORIC_LISTED, "listHistoric"
        );

        if (queryResult.code == CODE_HISTORIC_LISTED) {
            queryResult.message = queryResult.data.empty()
                ? "No hay acciones en el historial"
                : "Historial obtenido: " + to_string(queryResult.data.size()) + " entradas";
        }

        return queryResult;
    }
    catch (exception& e) {
        cout << "DB Exception: " << string(e.what());
        return { {}, CODE_EXCEPTION, "Excepcion no esperada en listHistoric" };
    }
    catch (...) {
        return { {}, CODE_EXCEPTION, "Excepcion desconocida en listHistoric" };
    }
}

DbResponse Connection::deleteLastHistoric() {
    try {
        string selectQuery = "SELECT ID FROM TB_HISTORIC ORDER BY STACK_POSITION DESC LIMIT 1;";

        sqlite3_stmt* selectStatement = nullptr;

        if (sqlite3_prepare_v2(db_, selectQuery.c_str(), -1, &selectStatement, nullptr) != SQLITE_OK) {
            return sqliteError(CODE_ERROR_DB, "deleteLastHistoric::select::prepare");
        }

        if (sqlite3_step(selectStatement) != SQLITE_ROW) {
            sqlite3_finalize(selectStatement);
            return { -1, CODE_HISTORIC_NOT_FOUND, "No hay acciones para deshacer" };
        }

        int lastHistoricId = sqlite3_column_int(selectStatement, 0);
        sqlite3_finalize(selectStatement);

        string deleteQuery = "DELETE FROM TB_HISTORIC WHERE ID = ?;";
        int resultCode = executeNonQuery(deleteQuery, { paramInt(lastHistoricId) });

        if (resultCode != SQLITE_DONE) {
            return sqliteError(CODE_ERROR_DB, "deleteLastHistoric::delete");
        }

        return { lastHistoricId, CODE_HISTORIC_DELETED, "Ultima accion eliminada del historial" };
    }
    catch (exception& e) {
        cout << "DB Exception: " << string(e.what());
        return { -1, CODE_EXCEPTION, "Excepcion no esperada en deleteLastHistoric" };
    }
    catch (...) {
        return { -1, CODE_EXCEPTION, "Excepcion desconocida en deleteLastHistoric" };
    }
}