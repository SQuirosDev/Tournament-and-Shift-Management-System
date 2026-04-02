#include "connection.h"
#include <iostream>

using namespace std;

// ------------------------------------------------------------
//  Helpers internos
// ------------------------------------------------------------

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


// ============================================================
//  Inicializacion de tablas
// ============================================================

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
            "  PLAYED_AT TEXT,"
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

        if (!execSimple(db_, "CREATE INDEX IF NOT EXISTS IDX_TEAM_TOURNAMENT ON TB_TEAM(TOURNAMENT_ID);"))
            return sqliteError(CODE_ERROR_DB_INIT, "initTables::IDX_TEAM_TOURNAMENT");

        if (!execSimple(db_, "CREATE INDEX IF NOT EXISTS IDX_PLAYER_TEAM ON TB_PLAYER(TEAM_ID);"))
            return sqliteError(CODE_ERROR_DB_INIT, "initTables::IDX_PLAYER_TEAM");

        if (!execSimple(db_, "CREATE INDEX IF NOT EXISTS IDX_MATCH_TOURNAMENT ON TB_MATCH(TOURNAMENT_ID, QUEUE_POSITION);"))
            return sqliteError(CODE_ERROR_DB_INIT, "initTables::IDX_MATCH_TOURNAMENT");

        if (!execSimple(db_, "CREATE INDEX IF NOT EXISTS IDX_MATCH_STATUS ON TB_MATCH(TOURNAMENT_ID, PHASE, STATUS);"))
            return sqliteError(CODE_ERROR_DB_INIT, "initTables::IDX_MATCH_STATUS");

        if (!execSimple(db_, "CREATE INDEX IF NOT EXISTS IDX_PETITION_QUEUE ON TB_PETITION(STATUS, QUEUE_POSITION);"))
            return sqliteError(CODE_ERROR_DB_INIT, "initTables::IDX_PETITION_QUEUE");

        if (!execSimple(db_, "CREATE INDEX IF NOT EXISTS IDX_HISTORIC_STACK ON TB_HISTORIC(TOURNAMENT_ID, STACK_POSITION DESC);"))
            return sqliteError(CODE_ERROR_DB_INIT, "initTables::IDX_HISTORIC_STACK");

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
        // Validacion: el nombre no puede estar vacio
        if (name.empty()) {
            return { -1, CODE_TOURNAMENT_INVALID_DATA, "El nombre del torneo no puede estar vacio" };
        }

        // Validacion: verificar que no exista un torneo con el mismo nombre
        string checkDuplicateQuery = "SELECT COUNT(*) FROM TB_TOURNAMENT WHERE NAME = '" + name + "';";
        if (rowExists(db_, checkDuplicateQuery)) {
            return { -1, CODE_TOURNAMENT_DUPLICATE, "Ya existe un torneo con el nombre: " + name };
        }

        // Definir la query con ? como placeholder para el valor de NAME
        const char* sqlQuery = "INSERT INTO TB_TOURNAMENT (NAME) VALUES (?);";
        sqlite3_stmt* sqlStatement = nullptr;

        // Compilar la query y dejarla lista para ejecutar
        if (sqlite3_prepare_v2(db_, sqlQuery, -1, &sqlStatement, nullptr) != SQLITE_OK) {
            return sqliteError(CODE_ERROR_DB, "insertTournament::prepare");
        }

        // Reemplazar el primer ? con el valor real de name
        sqlite3_bind_text(sqlStatement, 1, name.c_str(), -1, SQLITE_STATIC);

        // Ejecutar la query
        int resultCode = sqlite3_step(sqlStatement);

        // Liberar la memoria del statement (siempre se debe llamar)
        sqlite3_finalize(sqlStatement);

        // Verificar que el INSERT se ejecuto correctamente
        if (resultCode != SQLITE_DONE) {
            return sqliteError(CODE_ERROR_DB, "insertTournament::step");
        }

        // Obtener el ID que SQLite asigno automaticamente al nuevo registro
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
    DBQueryResponse<Tournament> queryResult;
    try {
        // Seleccionar todos los torneos ordenados por ID
        const char* sqlQuery = "SELECT ID, NAME, PHASE FROM TB_TOURNAMENT ORDER BY ID;";
        sqlite3_stmt* sqlStatement = nullptr;

        // Compilar la query
        if (sqlite3_prepare_v2(db_, sqlQuery, -1, &sqlStatement, nullptr) != SQLITE_OK) {
            DbResponse errorResponse = sqliteError(CODE_ERROR_DB, "listTournaments::prepare");
            queryResult.code = errorResponse.code;
            queryResult.message = errorResponse.message;
            return queryResult;
        }

        // Iterar sobre cada fila retornada
        while (sqlite3_step(sqlStatement) == SQLITE_ROW) {
            Tournament tournamentRow;

            // Mapear cada columna al campo correspondiente del struct
            tournamentRow.id = sqlite3_column_int(sqlStatement, 0);
            tournamentRow.name = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 1));
            tournamentRow.phase = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 2));

            // Agregar el registro a la lista de salida
            queryResult.data.push_back(tournamentRow);
        }

        sqlite3_finalize(sqlStatement);
        queryResult.code = CODE_TOURNAMENT_LISTED;
        if (queryResult.data.empty()) {
            queryResult.message = "No hay torneos registrados";
        }
        else {
            queryResult.message = "Torneos obtenidos: " + to_string(queryResult.data.size());
        }
        return queryResult;
    }
    catch (exception& e) {
        cout << "DB Exception: " << string(e.what());
        queryResult.code = CODE_EXCEPTION;
        queryResult.message = "Excepcion no esperada en listTournaments";
        return queryResult;
    }
    catch (...) {
        queryResult.code = CODE_EXCEPTION;
        queryResult.message = "Excepcion desconocida en listTournaments";
        return queryResult;
    }
}

DBQueryResponse<Tournament> Connection::obtainTournamentById(int id) {
    DBQueryResponse<Tournament> queryResult;
    try {
        // Buscar el torneo por su ID usando un placeholder ?
        const char* sqlQuery = "SELECT ID, NAME, PHASE FROM TB_TOURNAMENT WHERE ID = ?;";
        sqlite3_stmt* sqlStatement = nullptr;

        // Compilar la query
        if (sqlite3_prepare_v2(db_, sqlQuery, -1, &sqlStatement, nullptr) != SQLITE_OK) {
            DbResponse errorResponse = sqliteError(CODE_ERROR_DB, "obtainTournamentById::prepare");
            queryResult.code = errorResponse.code;
            queryResult.message = errorResponse.message;
            return queryResult;
        }

        // Reemplazar el ? con el ID recibido
        sqlite3_bind_int(sqlStatement, 1, id);

        // Si no retorna ninguna fila, el torneo no existe
        if (sqlite3_step(sqlStatement) != SQLITE_ROW) {
            sqlite3_finalize(sqlStatement);
            queryResult.code = CODE_TOURNAMENT_NOT_FOUND;
            queryResult.message = "Torneo con ID " + to_string(id) + " no encontrado";
            return queryResult;
        }

        // Mapear las columnas al struct de salida
        Tournament tournamentRow;
        tournamentRow.id = sqlite3_column_int(sqlStatement, 0);
        tournamentRow.name = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 1));
        tournamentRow.phase = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 2));

        sqlite3_finalize(sqlStatement);
        queryResult.data.push_back(tournamentRow);
        queryResult.code = CODE_TOURNAMENT_LISTED;
        queryResult.message = "Torneo encontrado";
        return queryResult;
    }
    catch (exception& e) {
        cout << "DB Exception: " << string(e.what());
        queryResult.code = CODE_EXCEPTION;
        queryResult.message = "Excepcion no esperada en obtainTournamentById";
        return queryResult;
    }
    catch (...) {
        queryResult.code = CODE_EXCEPTION;
        queryResult.message = "Excepcion desconocida en obtainTournamentById";
        return queryResult;
    }
}

DbResponse Connection::updateTournamentPhase(int id, string phase) {
    try {
        // Validacion: la fase debe ser uno de los valores permitidos por la BD
        if (phase != "Registro" && phase != "Grupos" && phase != "Eliminacion" && phase != "Finalizado") {
            return { -1, CODE_TOURNAMENT_INVALID_DATA, "Fase invalida. Use: Registro, Grupos, Eliminacion o Finalizado" };
        }

        // Validacion: verificar que el torneo exista
        string checkExistQuery = "SELECT COUNT(*) FROM TB_TOURNAMENT WHERE ID = " + to_string(id) + ";";
        if (!rowExists(db_, checkExistQuery)) {
            return { -1, CODE_TOURNAMENT_NOT_FOUND, "Torneo con ID " + to_string(id) + " no encontrado" };
        }

        // Actualizar solo el campo PHASE del torneo indicado
        const char* sqlQuery = "UPDATE TB_TOURNAMENT SET PHASE = ? WHERE ID = ?;";
        sqlite3_stmt* sqlStatement = nullptr;

        // Compilar la query
        if (sqlite3_prepare_v2(db_, sqlQuery, -1, &sqlStatement, nullptr) != SQLITE_OK) {
            return sqliteError(CODE_ERROR_DB, "updateTournamentPhase::prepare");
        }

        // Enlazar parametros: primer ? = phase, segundo ? = id
        sqlite3_bind_text(sqlStatement, 1, phase.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(sqlStatement, 2, id);

        int resultCode = sqlite3_step(sqlStatement);
        sqlite3_finalize(sqlStatement);

        if (resultCode != SQLITE_DONE) {
            return sqliteError(CODE_ERROR_DB, "updateTournamentPhase::step");
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
        // Validacion: el nombre no puede estar vacio
        if (newName.empty()) {
            return { -1, CODE_TOURNAMENT_INVALID_DATA, "El nombre no puede estar vacio" };
        }

        // Validacion: no debe existir otro torneo con el mismo nombre (excluyendo el actual)
        string checkDuplicateQuery = "SELECT COUNT(*) FROM TB_TOURNAMENT WHERE NAME = '" + newName + "' AND ID != " + to_string(id) + ";";
        if (rowExists(db_, checkDuplicateQuery)) {
            return { -1, CODE_TOURNAMENT_DUPLICATE, "Ya existe otro torneo con el nombre: " + newName };
        }

        // Validacion: verificar que el torneo a actualizar exista
        string checkExistQuery = "SELECT COUNT(*) FROM TB_TOURNAMENT WHERE ID = " + to_string(id) + ";";
        if (!rowExists(db_, checkExistQuery)) {
            return { -1, CODE_TOURNAMENT_NOT_FOUND, "Torneo con ID " + to_string(id) + " no encontrado" };
        }

        // Actualizar solo el campo NAME del torneo indicado
        const char* sqlQuery = "UPDATE TB_TOURNAMENT SET NAME = ? WHERE ID = ?;";
        sqlite3_stmt* sqlStatement = nullptr;

        // Compilar la query
        if (sqlite3_prepare_v2(db_, sqlQuery, -1, &sqlStatement, nullptr) != SQLITE_OK) {
            return sqliteError(CODE_ERROR_DB, "updateTournamentName::prepare");
        }

        // Enlazar parametros: primer ? = newName, segundo ? = id
        sqlite3_bind_text(sqlStatement, 1, newName.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(sqlStatement, 2, id);

        int resultCode = sqlite3_step(sqlStatement);
        sqlite3_finalize(sqlStatement);

        if (resultCode != SQLITE_DONE) {
            return sqliteError(CODE_ERROR_DB, "updateTournamentName::step");
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


// ============================================================
//  TB_TEAM
// ============================================================

DbResponse Connection::insertTeam(string name, int tournamentId) {
    try {
        // Validacion: el nombre no puede estar vacio
        if (name.empty()) {
            return { -1, CODE_TEAM_INVALID_DATA, "El nombre del equipo no puede estar vacio" };
        }

        // Validacion: el torneo al que pertenece el equipo debe existir
        string checkTournamentQuery = "SELECT COUNT(*) FROM TB_TOURNAMENT WHERE ID = " + to_string(tournamentId) + ";";
        if (!rowExists(db_, checkTournamentQuery)) {
            return { -1, CODE_TOURNAMENT_NOT_FOUND, "El torneo con ID " + to_string(tournamentId) + " no existe" };
        }

        // Validacion: no puede existir otro equipo con el mismo nombre en el mismo torneo
        string checkDuplicateQuery = "SELECT COUNT(*) FROM TB_TEAM WHERE NAME = '" + name + "' AND TOURNAMENT_ID = " + to_string(tournamentId) + ";";
        if (rowExists(db_, checkDuplicateQuery)) {
            return { -1, CODE_TEAM_DUPLICATE, "Ya existe un equipo con ese nombre en el torneo" };
        }

        // Insertar el equipo con su nombre y el ID del torneo
        const char* sqlQuery = "INSERT INTO TB_TEAM (NAME, TOURNAMENT_ID) VALUES (?, ?);";
        sqlite3_stmt* sqlStatement = nullptr;

        // Compilar la query
        if (sqlite3_prepare_v2(db_, sqlQuery, -1, &sqlStatement, nullptr) != SQLITE_OK) {
            return sqliteError(CODE_ERROR_DB, "insertTeam::prepare");
        }

        // Enlazar parametros: primer ? = name, segundo ? = tournamentId
        sqlite3_bind_text(sqlStatement, 1, name.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(sqlStatement, 2, tournamentId);

        int resultCode = sqlite3_step(sqlStatement);
        sqlite3_finalize(sqlStatement);

        if (resultCode != SQLITE_DONE) {
            return sqliteError(CODE_ERROR_DB, "insertTeam::step");
        }

        // Obtener el ID asignado automaticamente por SQLite
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
    DBQueryResponse<Team> queryResult;

    try {
        queryResult.data.clear();

        const char* sqlQuery = "SELECT ID, TOURNAMENT_ID, NAME, TOURNAMENTS, POINTS, WINS, DRAWS, LOSSES FROM TB_TEAM ORDER BY ID;";
        sqlite3_stmt* sqlStatement = nullptr;

        if (sqlite3_prepare_v2(db_, sqlQuery, -1, &sqlStatement, nullptr) != SQLITE_OK) {
            DbResponse errorResponse = sqliteError(CODE_ERROR_DB, "listAllTeams::prepare");
            queryResult.code = errorResponse.code;
            queryResult.message = errorResponse.message;
            return queryResult;
        }

        while (sqlite3_step(sqlStatement) == SQLITE_ROW) {
            Team teamRow;

            teamRow.id = sqlite3_column_int(sqlStatement, 0);
            teamRow.tournamentId = sqlite3_column_int(sqlStatement, 1);
            teamRow.name = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 2));
            teamRow.tournaments = sqlite3_column_int(sqlStatement, 3);
            teamRow.points = sqlite3_column_int(sqlStatement, 4);
            teamRow.wins = sqlite3_column_int(sqlStatement, 5);
            teamRow.draws = sqlite3_column_int(sqlStatement, 6);
            teamRow.losses = sqlite3_column_int(sqlStatement, 7);

            queryResult.data.push_back(teamRow);
        }

        sqlite3_finalize(sqlStatement);

        queryResult.code = CODE_TEAM_LISTED;

        if (queryResult.data.empty()) {
            queryResult.message = "No hay equipos registrados";
        }
        else {
            queryResult.message = "Equipos obtenidos: " + to_string(queryResult.data.size());
        }

        return queryResult;
    }
    catch (exception& e) {
        cout << "DB Exception: " << string(e.what());
        queryResult.code = CODE_EXCEPTION;
        queryResult.message = "Excepcion no esperada en listAllTeams";
        return queryResult;
    }
    catch (...) {
        queryResult.code = CODE_EXCEPTION;
        queryResult.message = "Excepcion desconocida en listAllTeams";
        return queryResult;
    }
}

DBQueryResponse<Team> Connection::listTeamsByTournament(int tournamentId) {
    DBQueryResponse<Team> queryResult;
    try {
        // Validacion: el torneo debe existir
        string checkTournamentQuery = "SELECT COUNT(*) FROM TB_TOURNAMENT WHERE ID = " + to_string(tournamentId) + ";";
        if (!rowExists(db_, checkTournamentQuery)) {
            queryResult.code = CODE_TOURNAMENT_NOT_FOUND;
            queryResult.message = "Torneo con ID " + to_string(tournamentId) + " no encontrado";
            return queryResult;
        }

        // Seleccionar todos los equipos del torneo con sus estadisticas
        const char* sqlQuery =
            "SELECT ID, TOURNAMENT_ID, NAME, TOURNAMENTS, POINTS, WINS, DRAWS, LOSSES "
            "FROM TB_TEAM WHERE TOURNAMENT_ID = ? ORDER BY ID;";
        sqlite3_stmt* sqlStatement = nullptr;

        // Compilar la query
        if (sqlite3_prepare_v2(db_, sqlQuery, -1, &sqlStatement, nullptr) != SQLITE_OK) {
            DbResponse errorResponse = sqliteError(CODE_ERROR_DB, "listTeamsByTournament::prepare");
            queryResult.code = errorResponse.code;
            queryResult.message = errorResponse.message;
            return queryResult;
        }

        // Reemplazar el ? con el ID del torneo
        sqlite3_bind_int(sqlStatement, 1, tournamentId);

        // Iterar sobre cada fila retornada
        while (sqlite3_step(sqlStatement) == SQLITE_ROW) {
            Team teamRow;

            // Mapear cada columna al campo correspondiente del struct
            teamRow.id = sqlite3_column_int(sqlStatement, 0);
            teamRow.tournamentId = sqlite3_column_int(sqlStatement, 1);
            teamRow.name = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 2));
            teamRow.tournaments = sqlite3_column_int(sqlStatement, 3);
            teamRow.points = sqlite3_column_int(sqlStatement, 4);
            teamRow.wins = sqlite3_column_int(sqlStatement, 5);
            teamRow.draws = sqlite3_column_int(sqlStatement, 6);
            teamRow.losses = sqlite3_column_int(sqlStatement, 7);

            queryResult.data.push_back(teamRow);
        }

        sqlite3_finalize(sqlStatement);
        queryResult.code = CODE_TEAM_LISTED;
        if (queryResult.data.empty()) {
            queryResult.message = "No hay equipos en este torneo";
        }
        else {
            queryResult.message = "Equipos obtenidos: " + to_string(queryResult.data.size());
        }
        return queryResult;
    }
    catch (exception& e) {
        cout << "DB Exception: " << string(e.what());
        queryResult.code = CODE_EXCEPTION;
        queryResult.message = "Excepcion no esperada en listTeamsByTournament";
        return queryResult;
    }
    catch (...) {
        queryResult.code = CODE_EXCEPTION;
        queryResult.message = "Excepcion desconocida en listTeamsByTournament";
        return queryResult;
    }
}

DBQueryResponse<Team> Connection::obtainTeamById(int id) {
    DBQueryResponse<Team> queryResult;
    try {
        // Seleccionar el equipo por su ID con todas sus estadisticas
        const char* sqlQuery =
            "SELECT ID, TOURNAMENT_ID, NAME, TOURNAMENTS, POINTS, WINS, DRAWS, LOSSES "
            "FROM TB_TEAM WHERE ID = ?;";
        sqlite3_stmt* sqlStatement = nullptr;

        // Compilar la query
        if (sqlite3_prepare_v2(db_, sqlQuery, -1, &sqlStatement, nullptr) != SQLITE_OK) {
            DbResponse errorResponse = sqliteError(CODE_ERROR_DB, "obtainTeamById::prepare");
            queryResult.code = errorResponse.code;
            queryResult.message = errorResponse.message;
            return queryResult;
        }

        // Reemplazar el ? con el ID recibido
        sqlite3_bind_int(sqlStatement, 1, id);

        // Si no retorna ninguna fila, el equipo no existe
        if (sqlite3_step(sqlStatement) != SQLITE_ROW) {
            sqlite3_finalize(sqlStatement);
            queryResult.code = CODE_TEAM_NOT_FOUND;
            queryResult.message = "Equipo con ID " + to_string(id) + " no encontrado";
            return queryResult;
        }

        // Mapear las columnas al struct de salida
        Team teamRow;
        teamRow.id = sqlite3_column_int(sqlStatement, 0);
        teamRow.tournamentId = sqlite3_column_int(sqlStatement, 1);
        teamRow.name = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 2));
        teamRow.tournaments = sqlite3_column_int(sqlStatement, 3);
        teamRow.points = sqlite3_column_int(sqlStatement, 4);
        teamRow.wins = sqlite3_column_int(sqlStatement, 5);
        teamRow.draws = sqlite3_column_int(sqlStatement, 6);
        teamRow.losses = sqlite3_column_int(sqlStatement, 7);

        sqlite3_finalize(sqlStatement);
        queryResult.data.push_back(teamRow);
        queryResult.code = CODE_TEAM_LISTED;
        queryResult.message = "Equipo encontrado";
        return queryResult;
    }
    catch (exception& e) {
        cout << "DB Exception: " << string(e.what());
        queryResult.code = CODE_EXCEPTION;
        queryResult.message = "Excepcion no esperada en obtainTeamById";
        return queryResult;
    }
    catch (...) {
        queryResult.code = CODE_EXCEPTION;
        queryResult.message = "Excepcion desconocida en obtainTeamById";
        return queryResult;
    }
}

DbResponse Connection::updateTeam(int id, string newName) {
    try {
        // Validacion: el nombre no puede estar vacio
        if (newName.empty()) {
            return { -1, CODE_TEAM_INVALID_DATA, "El nombre del equipo no puede estar vacio" };
        }

        // Validacion: verificar que el equipo exista
        string checkExistQuery = "SELECT COUNT(*) FROM TB_TEAM WHERE ID = " + to_string(id) + ";";
        if (!rowExists(db_, checkExistQuery)) {
            return { -1, CODE_TEAM_NOT_FOUND, "Equipo con ID " + to_string(id) + " no encontrado" };
        }

        // Actualizar solo el campo NAME del equipo indicado
        const char* sqlQuery = "UPDATE TB_TEAM SET NAME = ? WHERE ID = ?;";
        sqlite3_stmt* sqlStatement = nullptr;

        // Compilar la query
        if (sqlite3_prepare_v2(db_, sqlQuery, -1, &sqlStatement, nullptr) != SQLITE_OK) {
            return sqliteError(CODE_ERROR_DB, "updateTeam::prepare");
        }

        // Enlazar parametros: primer ? = newName, segundo ? = id
        sqlite3_bind_text(sqlStatement, 1, newName.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(sqlStatement, 2, id);

        int resultCode = sqlite3_step(sqlStatement);
        sqlite3_finalize(sqlStatement);

        if (resultCode != SQLITE_DONE) {
            return sqliteError(CODE_ERROR_DB, "updateTeam::step");
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

DbResponse Connection::updateTeamStats(int id, int points, int wins, int draws, int losses) {
    try {
        // Validacion: verificar que el equipo exista
        string checkExistQuery = "SELECT COUNT(*) FROM TB_TEAM WHERE ID = " + to_string(id) + ";";
        if (!rowExists(db_, checkExistQuery)) {
            return { -1, CODE_TEAM_NOT_FOUND, "Equipo con ID " + to_string(id) + " no encontrado" };
        }

        // Actualizar todas las estadisticas del equipo en una sola query
        const char* sqlQuery =
            "UPDATE TB_TEAM SET POINTS = ?, WINS = ?, DRAWS = ?, LOSSES = ? WHERE ID = ?;";
        sqlite3_stmt* sqlStatement = nullptr;

        // Compilar la query
        if (sqlite3_prepare_v2(db_, sqlQuery, -1, &sqlStatement, nullptr) != SQLITE_OK) {
            return sqliteError(CODE_ERROR_DB, "updateTeamStats::prepare");
        }

        // Enlazar cada estadistica en el orden de los ? en la query
        sqlite3_bind_int(sqlStatement, 1, points);
        sqlite3_bind_int(sqlStatement, 2, wins);
        sqlite3_bind_int(sqlStatement, 3, draws);
        sqlite3_bind_int(sqlStatement, 4, losses);
        sqlite3_bind_int(sqlStatement, 5, id);

        int resultCode = sqlite3_step(sqlStatement);
        sqlite3_finalize(sqlStatement);

        if (resultCode != SQLITE_DONE) {
            return sqliteError(CODE_ERROR_DB, "updateTeamStats::step");
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
        // Validacion: verificar que el equipo exista antes de eliminarlo
        string checkExistQuery = "SELECT COUNT(*) FROM TB_TEAM WHERE ID = " + to_string(id) + ";";
        if (!rowExists(db_, checkExistQuery)) {
            return { -1, CODE_TEAM_NOT_FOUND, "Equipo con ID " + to_string(id) + " no encontrado" };
        }

        // Eliminar el equipo por su ID
        // Por el ON DELETE CASCADE de la BD, sus jugadores se eliminan automaticamente
        const char* sqlQuery = "DELETE FROM TB_TEAM WHERE ID = ?;";
        sqlite3_stmt* sqlStatement = nullptr;

        // Compilar la query
        if (sqlite3_prepare_v2(db_, sqlQuery, -1, &sqlStatement, nullptr) != SQLITE_OK) {
            return sqliteError(CODE_ERROR_DB, "deleteTeam::prepare");
        }

        // Reemplazar el ? con el ID del equipo a eliminar
        sqlite3_bind_int(sqlStatement, 1, id);

        int resultCode = sqlite3_step(sqlStatement);
        sqlite3_finalize(sqlStatement);

        if (resultCode != SQLITE_DONE) {
            return sqliteError(CODE_ERROR_DB, "deleteTeam::step");
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

DbResponse Connection::insertPlayer(string name, int teamId) {
    try {
        // Validacion: el nombre no puede estar vacio
        if (name.empty()) {
            return { -1, CODE_PLAYER_INVALID_DATA, "El nombre del jugador no puede estar vacio" };
        }

        // Validacion: el equipo al que pertenece el jugador debe existir
        string checkTeamQuery = "SELECT COUNT(*) FROM TB_TEAM WHERE ID = " + to_string(teamId) + ";";
        if (!rowExists(db_, checkTeamQuery)) {
            return { -1, CODE_TEAM_NOT_FOUND, "El equipo con ID " + to_string(teamId) + " no existe" };
        }

        // Validacion: no puede existir otro jugador con el mismo nombre en el mismo equipo
        string checkDuplicateQuery = "SELECT COUNT(*) FROM TB_PLAYER WHERE NAME = '" + name + "' AND TEAM_ID = " + to_string(teamId) + ";";
        if (rowExists(db_, checkDuplicateQuery)) {
            return { -1, CODE_PLAYER_DUPLICATE, "Ya existe un jugador con ese nombre en el equipo" };
        }

        // Insertar el jugador con su nombre y el ID del equipo
        const char* sqlQuery = "INSERT INTO TB_PLAYER (NAME, TEAM_ID) VALUES (?, ?);";
        sqlite3_stmt* sqlStatement = nullptr;

        // Compilar la query
        if (sqlite3_prepare_v2(db_, sqlQuery, -1, &sqlStatement, nullptr) != SQLITE_OK) {
            return sqliteError(CODE_ERROR_DB, "insertPlayer::prepare");
        }

        // Enlazar parametros: primer ? = name, segundo ? = teamId
        sqlite3_bind_text(sqlStatement, 1, name.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(sqlStatement, 2, teamId);

        int resultCode = sqlite3_step(sqlStatement);
        sqlite3_finalize(sqlStatement);

        if (resultCode != SQLITE_DONE) {
            return sqliteError(CODE_ERROR_DB, "insertPlayer::step");
        }

        // Obtener el ID asignado automaticamente por SQLite
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
    DBQueryResponse<Player> queryResult;

    try {
        queryResult.data.clear();

        const char* sqlQuery = "SELECT ID, TEAM_ID, NAME FROM TB_PLAYER ORDER BY ID;";
        sqlite3_stmt* sqlStatement = nullptr;

        if (sqlite3_prepare_v2(db_, sqlQuery, -1, &sqlStatement, nullptr) != SQLITE_OK) {
            DbResponse errorResponse = sqliteError(CODE_ERROR_DB, "listAllPlayers::prepare");
            queryResult.code = errorResponse.code;
            queryResult.message = errorResponse.message;
            return queryResult;
        }

        while (sqlite3_step(sqlStatement) == SQLITE_ROW) {
            Player playerRow;

            playerRow.id = sqlite3_column_int(sqlStatement, 0);
            playerRow.teamId = sqlite3_column_int(sqlStatement, 1);
            playerRow.name = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 2));

            queryResult.data.push_back(playerRow);
        }

        sqlite3_finalize(sqlStatement);

        queryResult.code = CODE_PLAYER_LISTED;
        if (queryResult.data.empty()) {
            queryResult.message = "No hay jugadores registrados";
        }
        else {
            queryResult.message = "Jugadores obtenidos: " + to_string(queryResult.data.size());
        }

        return queryResult;
    }
    catch (exception& e) {
        cout << "DB Exception: " << string(e.what());
        queryResult.code = CODE_EXCEPTION;
        queryResult.message = "Excepcion no esperada en listAllPlayers";
        return queryResult;
    }
    catch (...) {
        queryResult.code = CODE_EXCEPTION;
        queryResult.message = "Excepcion desconocida en listAllPlayers";
        return queryResult;
    }
}

DBQueryResponse<Player> Connection::listPlayersByTeam(int teamId) {
    DBQueryResponse<Player> queryResult;
    try {
        // Limpiar la lista antes de llenarla
        queryResult.data.clear();

        // Validacion: el equipo debe existir
        string checkTeamQuery = "SELECT COUNT(*) FROM TB_TEAM WHERE ID = " + to_string(teamId) + ";";
        if (!rowExists(db_, checkTeamQuery)) {
            queryResult.code = CODE_TEAM_NOT_FOUND;
            queryResult.message = "Equipo con ID " + to_string(teamId) + " no encontrado";
            return queryResult;
        }

        // Seleccionar todos los jugadores del equipo ordenados por ID
        const char* sqlQuery = "SELECT ID, TEAM_ID, NAME FROM TB_PLAYER WHERE TEAM_ID = ? ORDER BY ID;";
        sqlite3_stmt* sqlStatement = nullptr;

        // Compilar la query
        if (sqlite3_prepare_v2(db_, sqlQuery, -1, &sqlStatement, nullptr) != SQLITE_OK) {
            DbResponse errorResponse = sqliteError(CODE_ERROR_DB, "listPlayersByTeam::prepare");
            queryResult.code = errorResponse.code;
            queryResult.message = errorResponse.message;
            return queryResult;
        }

        // Reemplazar el ? con el ID del equipo
        sqlite3_bind_int(sqlStatement, 1, teamId);

        // Iterar sobre cada fila retornada
        while (sqlite3_step(sqlStatement) == SQLITE_ROW) {
            Player playerRow;

            // Mapear cada columna al campo correspondiente del struct
            playerRow.id = sqlite3_column_int(sqlStatement, 0);
            playerRow.teamId = sqlite3_column_int(sqlStatement, 1);
            playerRow.name = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 2));

            queryResult.data.push_back(playerRow);
        }

        sqlite3_finalize(sqlStatement);
        queryResult.code = CODE_PLAYER_LISTED;
        if (queryResult.data.empty()) {
            queryResult.message = "No hay jugadores en este equipo";
        }
        else {
            queryResult.message = "Jugadores obtenidos: " + to_string(queryResult.data.size());
        }
        return queryResult;
    }
    catch (exception& e) {
        cout << "DB Exception: " << string(e.what());
        queryResult.code = CODE_EXCEPTION;
        queryResult.message = "Excepcion no esperada en listPlayersByTeam";
        return queryResult;
    }
    catch (...) {
        queryResult.code = CODE_EXCEPTION;
        queryResult.message = "Excepcion desconocida en listPlayersByTeam";
        return queryResult;
    }
}

DBQueryResponse<Player> Connection::obtainPlayerById(int id) {
    DBQueryResponse<Player> queryResult;
    try {
        // Buscar el jugador por su ID
        const char* sqlQuery = "SELECT ID, TEAM_ID, NAME FROM TB_PLAYER WHERE ID = ?;";
        sqlite3_stmt* sqlStatement = nullptr;

        // Compilar la query
        if (sqlite3_prepare_v2(db_, sqlQuery, -1, &sqlStatement, nullptr) != SQLITE_OK) {
            DbResponse errorResponse = sqliteError(CODE_ERROR_DB, "obtainPlayerById::prepare");
            queryResult.code = errorResponse.code;
            queryResult.message = errorResponse.message;
            return queryResult;
        }

        // Reemplazar el ? con el ID recibido
        sqlite3_bind_int(sqlStatement, 1, id);

        // Si no retorna ninguna fila, el jugador no existe
        if (sqlite3_step(sqlStatement) != SQLITE_ROW) {
            sqlite3_finalize(sqlStatement);
            queryResult.code = CODE_PLAYER_NOT_FOUND;
            queryResult.message = "Jugador con ID " + to_string(id) + " no encontrado";
            return queryResult;
        }

        // Mapear las columnas al struct de salida
        Player playerRow;
        playerRow.id = sqlite3_column_int(sqlStatement, 0);
        playerRow.teamId = sqlite3_column_int(sqlStatement, 1);
        playerRow.name = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 2));

        sqlite3_finalize(sqlStatement);
        queryResult.data.push_back(playerRow);
        queryResult.code = CODE_PLAYER_LISTED;
        queryResult.message = "Jugador encontrado";
        return queryResult;
    }
    catch (exception& e) {
        cout << "DB Exception: " << string(e.what());
        queryResult.code = CODE_EXCEPTION;
        queryResult.message = "Excepcion no esperada en obtainPlayerById";
        return queryResult;
    }
    catch (...) {
        queryResult.code = CODE_EXCEPTION;
        queryResult.message = "Excepcion desconocida en obtainPlayerById";
        return queryResult;
    }
}

DbResponse Connection::updatePlayer(int id, string name) {
    try {
        // Validacion: el nombre no puede estar vacio
        if (name.empty()) {
            return { -1, CODE_PLAYER_INVALID_DATA, "El nombre del jugador no puede estar vacio" };
        }

        // Validacion: verificar que el jugador exista
        string checkExistQuery = "SELECT COUNT(*) FROM TB_PLAYER WHERE ID = " + to_string(id) + ";";
        if (!rowExists(db_, checkExistQuery)) {
            return { -1, CODE_PLAYER_NOT_FOUND, "Jugador con ID " + to_string(id) + " no encontrado" };
        }

        // Actualizar solo el campo NAME del jugador indicado
        const char* sqlQuery = "UPDATE TB_PLAYER SET NAME = ? WHERE ID = ?;";
        sqlite3_stmt* sqlStatement = nullptr;

        // Compilar la query
        if (sqlite3_prepare_v2(db_, sqlQuery, -1, &sqlStatement, nullptr) != SQLITE_OK) {
            return sqliteError(CODE_ERROR_DB, "updatePlayer::prepare");
        }

        // Enlazar parametros: primer ? = name, segundo ? = id
        sqlite3_bind_text(sqlStatement, 1, name.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(sqlStatement, 2, id);

        int resultCode = sqlite3_step(sqlStatement);
        sqlite3_finalize(sqlStatement);

        if (resultCode != SQLITE_DONE) {
            return sqliteError(CODE_ERROR_DB, "updatePlayer::step");
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
        // Validacion: verificar que el jugador exista
        string checkPlayerQuery = "SELECT COUNT(*) FROM TB_PLAYER WHERE ID = " + to_string(playerId) + ";";
        if (!rowExists(db_, checkPlayerQuery)) {
            return { -1, CODE_PLAYER_NOT_FOUND, "Jugador con ID " + to_string(playerId) + " no encontrado" };
        }

        const char* sqlQuery = "UPDATE TB_PLAYER SET TEAM_ID = ? WHERE ID = ?;";
        sqlite3_stmt* sqlStatement = nullptr;

        if (sqlite3_prepare_v2(db_, sqlQuery, -1, &sqlStatement, nullptr) != SQLITE_OK) {
            return sqliteError(CODE_ERROR_DB, "updatePlayerTeam::prepare");
        }

        sqlite3_bind_int(sqlStatement, 1, teamId);
        sqlite3_bind_int(sqlStatement, 2, playerId);

        if (sqlite3_step(sqlStatement) != SQLITE_DONE) {
            sqlite3_finalize(sqlStatement);
            return sqliteError(CODE_ERROR_DB, "updatePlayerTeam::step");
        }

        sqlite3_finalize(sqlStatement);

        return { 1, CODE_PLAYER_UPDATED, "Equipo del jugador actualizado correctamente" };
    }
    catch (exception& e) {
        cout << "DB Exception: " << string(e.what());
        return { -1, CODE_EXCEPTION, "Excepcion en updatePlayerTeam" };
    }
    catch (...) {
        return { -1, CODE_EXCEPTION, "Excepcion desconocida en updatePlayerTeam" };
    }
}

DbResponse Connection::deletePlayer(int id) {
    try {
        // Validacion: verificar que el jugador exista antes de eliminarlo
        string checkExistQuery = "SELECT COUNT(*) FROM TB_PLAYER WHERE ID = " + to_string(id) + ";";
        if (!rowExists(db_, checkExistQuery)) {
            return { -1, CODE_PLAYER_NOT_FOUND, "Jugador con ID " + to_string(id) + " no encontrado" };
        }

        // Eliminar el jugador por su ID
        const char* sqlQuery = "DELETE FROM TB_PLAYER WHERE ID = ?;";
        sqlite3_stmt* sqlStatement = nullptr;

        // Compilar la query
        if (sqlite3_prepare_v2(db_, sqlQuery, -1, &sqlStatement, nullptr) != SQLITE_OK) {
            return sqliteError(CODE_ERROR_DB, "deletePlayer::prepare");
        }

        // Reemplazar el ? con el ID del jugador a eliminar
        sqlite3_bind_int(sqlStatement, 1, id);

        int resultCode = sqlite3_step(sqlStatement);
        sqlite3_finalize(sqlStatement);

        if (resultCode != SQLITE_DONE) {
            return sqliteError(CODE_ERROR_DB, "deletePlayer::step");
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

DbResponse Connection::insertMatch(int tournamentId, int teamAId, int teamBId, string phase, int round, int queuePosition) {
    try {
        // Validacion: la fase debe ser uno de los valores permitidos
        if (phase != "Grupos" && phase != "Eliminacion") {
            return { -1, CODE_MATCH_INVALID_DATA, "Fase invalida. Use: Grupos o Eliminacion" };
        }

        // Validacion: un equipo no puede jugar contra si mismo
        if (teamAId == teamBId) {
            return { -1, CODE_MATCH_INVALID_DATA, "Un equipo no puede jugar contra si mismo" };
        }

        // Validacion: el torneo debe existir
        string checkTournamentQuery = "SELECT COUNT(*) FROM TB_TOURNAMENT WHERE ID = " + to_string(tournamentId) + ";";
        if (!rowExists(db_, checkTournamentQuery)) {
            return { -1, CODE_TOURNAMENT_NOT_FOUND, "El torneo con ID " + to_string(tournamentId) + " no existe" };
        }

        // Insertar el partido con los datos basicos, el resultado se registra despues con updateMatchResult
        const char* sqlQuery =
            "INSERT INTO TB_MATCH (TOURNAMENT_ID, TEAM_A_ID, TEAM_B_ID, PHASE, ROUND, QUEUE_POSITION) "
            "VALUES (?, ?, ?, ?, ?, ?);";
        sqlite3_stmt* sqlStatement = nullptr;

        // Compilar la query
        if (sqlite3_prepare_v2(db_, sqlQuery, -1, &sqlStatement, nullptr) != SQLITE_OK) {
            return sqliteError(CODE_ERROR_DB, "insertMatch::prepare");
        }

        // Enlazar cada parametro en el orden de los ? en la query
        sqlite3_bind_int(sqlStatement, 1, tournamentId);
        sqlite3_bind_int(sqlStatement, 2, teamAId);
        sqlite3_bind_int(sqlStatement, 3, teamBId);
        sqlite3_bind_text(sqlStatement, 4, phase.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(sqlStatement, 5, round);
        sqlite3_bind_int(sqlStatement, 6, queuePosition);

        int resultCode = sqlite3_step(sqlStatement);
        sqlite3_finalize(sqlStatement);

        if (resultCode != SQLITE_DONE) {
            return sqliteError(CODE_ERROR_DB, "insertMatch::step");
        }

        // Obtener el ID asignado automaticamente por SQLite
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
    DBQueryResponse<Match> queryResult;
    try {
        // Limpiar la lista antes de llenarla
        queryResult.data.clear();

        // Validacion: el torneo debe existir
        string checkTournamentQuery = "SELECT COUNT(*) FROM TB_TOURNAMENT WHERE ID = " + to_string(tournamentId) + ";";
        if (!rowExists(db_, checkTournamentQuery)) {
            queryResult.code = CODE_TOURNAMENT_NOT_FOUND;
            queryResult.message = "Torneo con ID " + to_string(tournamentId) + " no encontrado";
            return queryResult;
        }

        // IFNULL maneja columnas que pueden ser NULL (WINNER_ID, RESULT, PLAYED_AT)
        // y los convierte a un valor por defecto para evitar errores al mapear
        const char* sqlQuery =
            "SELECT ID, TOURNAMENT_ID, TEAM_A_ID, TEAM_B_ID, PHASE, ROUND, STATUS, "
            "IFNULL(WINNER_ID, 0), IFNULL(RESULT, ''), QUEUE_POSITION, IFNULL(PLAYED_AT, '') "
            "FROM TB_MATCH WHERE TOURNAMENT_ID = ? ORDER BY QUEUE_POSITION;";
        sqlite3_stmt* sqlStatement = nullptr;

        // Compilar la query
        if (sqlite3_prepare_v2(db_, sqlQuery, -1, &sqlStatement, nullptr) != SQLITE_OK) {
            DbResponse errorResponse = sqliteError(CODE_ERROR_DB, "listMatchesByTournament::prepare");
            queryResult.code = errorResponse.code;
            queryResult.message = errorResponse.message;
            return queryResult;
        }

        // Reemplazar el ? con el ID del torneo
        sqlite3_bind_int(sqlStatement, 1, tournamentId);

        // Iterar sobre cada fila retornada
        while (sqlite3_step(sqlStatement) == SQLITE_ROW) {
            Match matchRow;

            // Mapear cada columna al campo correspondiente del struct
            matchRow.id = sqlite3_column_int(sqlStatement, 0);
            matchRow.tournamentId = sqlite3_column_int(sqlStatement, 1);
            matchRow.teamAId = sqlite3_column_int(sqlStatement, 2);
            matchRow.teamBId = sqlite3_column_int(sqlStatement, 3);
            matchRow.phase = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 4));
            matchRow.round = sqlite3_column_int(sqlStatement, 5);
            matchRow.status = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 6));
            matchRow.winnerId = sqlite3_column_int(sqlStatement, 7);
            matchRow.result = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 8));
            matchRow.queuePosition = sqlite3_column_int(sqlStatement, 9);
            matchRow.playedAt = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 10));

            queryResult.data.push_back(matchRow);
        }

        sqlite3_finalize(sqlStatement);
        queryResult.code = CODE_MATCH_LISTED;
        if (queryResult.data.empty()) {
            queryResult.message = "No hay partidos en este torneo";
        }
        else {
            queryResult.message = "Partidos obtenidos: " + to_string(queryResult.data.size());
        }
        return queryResult;
    }
    catch (exception& e) {
        cout << "DB Exception: " << string(e.what());
        queryResult.code = CODE_EXCEPTION;
        queryResult.message = "Excepcion no esperada en listMatchesByTournament";
        return queryResult;
    }
    catch (...) {
        queryResult.code = CODE_EXCEPTION;
        queryResult.message = "Excepcion desconocida en listMatchesByTournament";
        return queryResult;
    }
}

DBQueryResponse<Match> Connection::listMatchesByPhase(int tournamentId, string phase) {
    DBQueryResponse<Match> queryResult;
    try {
        // Limpiar la lista antes de llenarla
        queryResult.data.clear();

        // Validacion: la fase debe ser valida
        if (phase != "Grupos" && phase != "Eliminacion") {
            queryResult.code = CODE_MATCH_INVALID_DATA;
            queryResult.message = "Fase invalida. Use: Grupos o Eliminacion";
            return queryResult;
        }

        // Validacion: el torneo debe existir
        string checkTournamentQuery = "SELECT COUNT(*) FROM TB_TOURNAMENT WHERE ID = " + to_string(tournamentId) + ";";
        if (!rowExists(db_, checkTournamentQuery)) {
            queryResult.code = CODE_TOURNAMENT_NOT_FOUND;
            queryResult.message = "Torneo con ID " + to_string(tournamentId) + " no encontrado";
            return queryResult;
        }

        // Filtrar partidos por torneo Y por fase especifica
        const char* sqlQuery =
            "SELECT ID, TOURNAMENT_ID, TEAM_A_ID, TEAM_B_ID, PHASE, ROUND, STATUS, "
            "IFNULL(WINNER_ID, 0), IFNULL(RESULT, ''), QUEUE_POSITION, IFNULL(PLAYED_AT, '') "
            "FROM TB_MATCH WHERE TOURNAMENT_ID = ? AND PHASE = ? ORDER BY QUEUE_POSITION;";
        sqlite3_stmt* sqlStatement = nullptr;

        // Compilar la query
        if (sqlite3_prepare_v2(db_, sqlQuery, -1, &sqlStatement, nullptr) != SQLITE_OK) {
            DbResponse errorResponse = sqliteError(CODE_ERROR_DB, "listMatchesByPhase::prepare");
            queryResult.code = errorResponse.code;
            queryResult.message = errorResponse.message;
            return queryResult;
        }

        // Enlazar parametros: primer ? = tournamentId, segundo ? = phase
        sqlite3_bind_int(sqlStatement, 1, tournamentId);
        sqlite3_bind_text(sqlStatement, 2, phase.c_str(), -1, SQLITE_STATIC);

        // Iterar sobre cada fila retornada
        while (sqlite3_step(sqlStatement) == SQLITE_ROW) {
            Match matchRow;
            matchRow.id = sqlite3_column_int(sqlStatement, 0);
            matchRow.tournamentId = sqlite3_column_int(sqlStatement, 1);
            matchRow.teamAId = sqlite3_column_int(sqlStatement, 2);
            matchRow.teamBId = sqlite3_column_int(sqlStatement, 3);
            matchRow.phase = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 4));
            matchRow.round = sqlite3_column_int(sqlStatement, 5);
            matchRow.status = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 6));
            matchRow.winnerId = sqlite3_column_int(sqlStatement, 7);
            matchRow.result = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 8));
            matchRow.queuePosition = sqlite3_column_int(sqlStatement, 9);
            matchRow.playedAt = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 10));
            queryResult.data.push_back(matchRow);
        }

        sqlite3_finalize(sqlStatement);
        queryResult.code = CODE_MATCH_LISTED;
        if (queryResult.data.empty()) {
            queryResult.message = "No hay partidos en la fase: " + phase;
        }
        else {
            queryResult.message = "Partidos obtenidos: " + to_string(queryResult.data.size());
        }
        return queryResult;
    }
    catch (exception& e) {
        cout << "DB Exception: " << string(e.what());
        queryResult.code = CODE_EXCEPTION;
        queryResult.message = "Excepcion no esperada en listMatchesByPhase";
        return queryResult;
    }
    catch (...) {
        queryResult.code = CODE_EXCEPTION;
        queryResult.message = "Excepcion desconocida en listMatchesByPhase";
        return queryResult;
    }
}

DBQueryResponse<Match> Connection::obtainNextMatch(int tournamentId) {
    DBQueryResponse<Match> queryResult;
    try {
        // Validacion: el torneo debe existir
        string checkTournamentQuery = "SELECT COUNT(*) FROM TB_TOURNAMENT WHERE ID = " + to_string(tournamentId) + ";";
        if (!rowExists(db_, checkTournamentQuery)) {
            queryResult.code = CODE_TOURNAMENT_NOT_FOUND;
            queryResult.message = "Torneo con ID " + to_string(tournamentId) + " no encontrado";
            return queryResult;
        }

        // Obtener el partido pendiente con menor QUEUE_POSITION (frente de la cola)
        const char* sqlQuery =
            "SELECT ID, TOURNAMENT_ID, TEAM_A_ID, TEAM_B_ID, PHASE, ROUND, STATUS, "
            "IFNULL(WINNER_ID, 0), IFNULL(RESULT, ''), QUEUE_POSITION, IFNULL(PLAYED_AT, '') "
            "FROM TB_MATCH WHERE TOURNAMENT_ID = ? AND STATUS = 'Pendiente' "
            "ORDER BY QUEUE_POSITION LIMIT 1;";
        sqlite3_stmt* sqlStatement = nullptr;

        // Compilar la query
        if (sqlite3_prepare_v2(db_, sqlQuery, -1, &sqlStatement, nullptr) != SQLITE_OK) {
            DbResponse errorResponse = sqliteError(CODE_ERROR_DB, "obtainNextMatch::prepare");
            queryResult.code = errorResponse.code;
            queryResult.message = errorResponse.message;
            return queryResult;
        }

        // Reemplazar el ? con el ID del torneo
        sqlite3_bind_int(sqlStatement, 1, tournamentId);

        // Si no retorna ninguna fila, no hay partidos pendientes
        if (sqlite3_step(sqlStatement) != SQLITE_ROW) {
            sqlite3_finalize(sqlStatement);
            queryResult.code = CODE_MATCH_NOT_FOUND;
            queryResult.message = "No hay partidos pendientes en el torneo";
            return queryResult;
        }

        // Mapear las columnas al struct de salida
        Match matchRow;
        matchRow.id = sqlite3_column_int(sqlStatement, 0);
        matchRow.tournamentId = sqlite3_column_int(sqlStatement, 1);
        matchRow.teamAId = sqlite3_column_int(sqlStatement, 2);
        matchRow.teamBId = sqlite3_column_int(sqlStatement, 3);
        matchRow.phase = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 4));
        matchRow.round = sqlite3_column_int(sqlStatement, 5);
        matchRow.status = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 6));
        matchRow.winnerId = sqlite3_column_int(sqlStatement, 7);
        matchRow.result = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 8));
        matchRow.queuePosition = sqlite3_column_int(sqlStatement, 9);
        matchRow.playedAt = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 10));

        sqlite3_finalize(sqlStatement);
        queryResult.data.push_back(matchRow);
        queryResult.code = CODE_MATCH_LISTED;
        queryResult.message = "Siguiente partido obtenido";
        return queryResult;
    }
    catch (exception& e) {
        cout << "DB Exception: " << string(e.what());
        queryResult.code = CODE_EXCEPTION;
        queryResult.message = "Excepcion no esperada en obtainNextMatch";
        return queryResult;
    }
    catch (...) {
        queryResult.code = CODE_EXCEPTION;
        queryResult.message = "Excepcion desconocida en obtainNextMatch";
        return queryResult;
    }
}

DbResponse Connection::updateMatchResult(int id, string result, int winnerId, int round) {
    try {
        // Validacion: el resultado debe ser uno de los valores permitidos
        if (result != "Gana A" && result != "Gana B" && result != "Empate") {
            return { -1, CODE_MATCH_INVALID_DATA, "Resultado invalido. Use: Gana A, Gana B o Empate" };
        }

        // Validacion: verificar que el partido exista
        string checkExistQuery = "SELECT COUNT(*) FROM TB_MATCH WHERE ID = " + to_string(id) + ";";
        if (!rowExists(db_, checkExistQuery)) {
            return { -1, CODE_MATCH_NOT_FOUND, "Partido con ID " + to_string(id) + " no encontrado" };
        }

        // Validacion: el partido debe estar pendiente, no se puede modificar uno ya finalizado
        string checkPendingQuery = "SELECT COUNT(*) FROM TB_MATCH WHERE ID = " + to_string(id) + " AND STATUS = 'Pendiente';";
        if (!rowExists(db_, checkPendingQuery)) {
            return { -1, CODE_MATCH_INVALID_DATA, "El partido ya fue finalizado" };
        }

        // Actualizar resultado, ganador, ronda, cambiar estado a Finalizado y registrar fecha/hora automaticamente
        const char* sqlQuery =
            "UPDATE TB_MATCH SET ROUND = ?, RESULT = ?, WINNER_ID = ?, STATUS = 'Finalizado', PLAYED_AT = datetime('now') "
            "WHERE ID = ?;";
        sqlite3_stmt* sqlStatement = nullptr;

        // Compilar la query
        if (sqlite3_prepare_v2(db_, sqlQuery, -1, &sqlStatement, nullptr) != SQLITE_OK) {
            return sqliteError(CODE_ERROR_DB, "updateMatchResult::prepare");
        }

        // Enlazar parametros en el orden de los ? en la query
        sqlite3_bind_int(sqlStatement, 1, round);
        sqlite3_bind_text(sqlStatement, 2, result.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(sqlStatement, 3, winnerId);
        sqlite3_bind_int(sqlStatement, 4, id);

        int resultCode = sqlite3_step(sqlStatement);
        sqlite3_finalize(sqlStatement);

        if (resultCode != SQLITE_DONE) {
            return sqliteError(CODE_ERROR_DB, "updateMatchResult::step");
        }

        return { id, CODE_MATCH_UPDATED, "Resultado del partido registrado exitosamente" };
    }
    catch (exception& e) {
        cout << "DB Exception: " << string(e.what());
        return { -1, CODE_EXCEPTION, "Excepcion no esperada en updateMatchResult" };
    }
    catch (...) {
        return { -1, CODE_EXCEPTION, "Excepcion desconocida en updateMatchResult" };
    }
}

DbResponse Connection::deleteMatch(int id) {
    try {
        // Validacion: verificar que el partido exista antes de eliminarlo
        string checkExistQuery = "SELECT COUNT(*) FROM TB_MATCH WHERE ID = " + to_string(id) + ";";
        if (!rowExists(db_, checkExistQuery)) {
            return { -1, CODE_MATCH_NOT_FOUND, "Partido con ID " + to_string(id) + " no encontrado" };
        }

        // Eliminar el partido por su ID
        const char* sqlQuery = "DELETE FROM TB_MATCH WHERE ID = ?;";
        sqlite3_stmt* sqlStatement = nullptr;

        // Compilar la query
        if (sqlite3_prepare_v2(db_, sqlQuery, -1, &sqlStatement, nullptr) != SQLITE_OK) {
            return sqliteError(CODE_ERROR_DB, "deleteMatch::prepare");
        }

        // Reemplazar el ? con el ID del partido a eliminar
        sqlite3_bind_int(sqlStatement, 1, id);

        int resultCode = sqlite3_step(sqlStatement);
        sqlite3_finalize(sqlStatement);

        if (resultCode != SQLITE_DONE) {
            return sqliteError(CODE_ERROR_DB, "deleteMatch::step");
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

DbResponse Connection::insertPetition(string requesterName, string type, string description) {
    try {
        // Validacion: el nombre del solicitante no puede estar vacio
        if (requesterName.empty()) {
            return { -1, CODE_PETITION_INVALID_DATA, "El nombre del solicitante no puede estar vacio" };
        }

        // Validacion: el tipo debe ser uno de los valores permitidos
        if (type != "Inscripcion" && type != "Consulta" && type != "Apelacion") {
            return { -1, CODE_PETITION_INVALID_DATA, "Tipo invalido. Use: Inscripcion, Consulta o Apelacion" };
        }

        // Calcular el siguiente QUEUE_POSITION automaticamente
        // IFNULL maneja el caso cuando la tabla esta vacia (MAX seria NULL)
        string checkQueueQuery = "SELECT IFNULL(MAX(QUEUE_POSITION), 0) + 1 FROM TB_PETITION;";
        sqlite3_stmt* sqlStatementQueue = nullptr;
        int nextQueuePosition = 1;

        if (sqlite3_prepare_v2(db_, checkQueueQuery.c_str(), -1, &sqlStatementQueue, nullptr) == SQLITE_OK) {
            if (sqlite3_step(sqlStatementQueue) == SQLITE_ROW) {
                nextQueuePosition = sqlite3_column_int(sqlStatementQueue, 0);
            }
        }
        sqlite3_finalize(sqlStatementQueue);

        // Insertar la peticion con su posicion en la cola (FIFO)
        const char* sqlQuery =
            "INSERT INTO TB_PETITION (REQUESTER_NAME, TYPE, DESCRIPTION, QUEUE_POSITION) VALUES (?, ?, ?, ?);";
        sqlite3_stmt* sqlStatement = nullptr;

        // Compilar la query
        if (sqlite3_prepare_v2(db_, sqlQuery, -1, &sqlStatement, nullptr) != SQLITE_OK) {
            return sqliteError(CODE_ERROR_DB, "insertPetition::prepare");
        }

        // Enlazar cada parametro en el orden de los ? en la query
        sqlite3_bind_text(sqlStatement, 1, requesterName.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(sqlStatement, 2, type.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(sqlStatement, 3, description.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(sqlStatement, 4, nextQueuePosition);

        int resultCode = sqlite3_step(sqlStatement);
        sqlite3_finalize(sqlStatement);

        if (resultCode != SQLITE_DONE) {
            return sqliteError(CODE_ERROR_DB, "insertPetition::step");
        }

        // Obtener el ID asignado automaticamente por SQLite
        int insertedId = static_cast<int>(sqlite3_last_insert_rowid(db_));
        return { insertedId, CODE_PETITION_CREATED, "Peticion encolada exitosamente" };
    }
    catch (exception& e) {
        cout << "DB Exception: " << string(e.what());
        return { -1, CODE_EXCEPTION, "Excepcion no esperada en insertPetition" };
    }
    catch (...) {
        return { -1, CODE_EXCEPTION, "Excepcion desconocida en insertPetition" };
    }
}

DBQueryResponse<Petition> Connection::listPendingPetitions() {
    DBQueryResponse<Petition> queryResult;
    try {
        // Seleccionar solo las peticiones PENDIENTES en orden FIFO (menor QUEUE_POSITION primero)
        const char* sqlQuery =
            "SELECT ID, REQUESTER_NAME, TYPE, DESCRIPTION, RESPONSE, STATUS, QUEUE_POSITION "
            "FROM TB_PETITION WHERE STATUS = 'Pendiente' ORDER BY QUEUE_POSITION;";
        sqlite3_stmt* sqlStatement = nullptr;

        // Compilar la query
        if (sqlite3_prepare_v2(db_, sqlQuery, -1, &sqlStatement, nullptr) != SQLITE_OK) {
            DbResponse errorResponse = sqliteError(CODE_ERROR_DB, "listPendingPetitions::prepare");
            queryResult.code = errorResponse.code;
            queryResult.message = errorResponse.message;
            return queryResult;
        }

        // Iterar sobre cada fila retornada
        while (sqlite3_step(sqlStatement) == SQLITE_ROW) {
            Petition petitionRow;

            // Mapear cada columna al campo correspondiente del struct
            petitionRow.id = sqlite3_column_int(sqlStatement, 0);
            petitionRow.requesterName = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 1));
            petitionRow.type = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 2));
            petitionRow.description = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 3));
            petitionRow.status = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 4));
            petitionRow.queuePosition = sqlite3_column_int(sqlStatement, 5);

            queryResult.data.push_back(petitionRow);
        }

        sqlite3_finalize(sqlStatement);
        queryResult.code = CODE_PETITION_LISTED;
        if (queryResult.data.empty()) {
            queryResult.message = "No hay peticiones pendientes";
        }
        else {
            queryResult.message = "Peticiones pendientes: " + to_string(queryResult.data.size());
        }
        return queryResult;
    }
    catch (exception& e) {
        cout << "DB Exception: " << string(e.what());
        queryResult.code = CODE_EXCEPTION;
        queryResult.message = "Excepcion no esperada en listPendingPetitions";
        return queryResult;
    }
    catch (...) {
        queryResult.code = CODE_EXCEPTION;
        queryResult.message = "Excepcion desconocida en listPendingPetitions";
        return queryResult;
    }
}

DBQueryResponse<Petition> Connection::obtainPetitionById(int id) {
    DBQueryResponse<Petition> queryResult;
    try {
        // Buscar la peticion por su ID
        const char* sqlQuery = "SELECT ID, REQUESTER_NAME, TYPE, DESCRIPTION, RESPONSE, STATUS, QUEUE_POSITION FROM TB_PETITION WHERE ID = ?;";
        sqlite3_stmt* sqlStatement = nullptr;

        // Compilar la query
        if (sqlite3_prepare_v2(db_, sqlQuery, -1, &sqlStatement, nullptr) != SQLITE_OK) {
            DbResponse errorResponse = sqliteError(CODE_ERROR_DB, "obtainPetitionById::prepare");
            queryResult.code = errorResponse.code;
            queryResult.message = errorResponse.message;
            return queryResult;
        }

        // Reemplazar el ? con el ID recibido
        sqlite3_bind_int(sqlStatement, 1, id);

        // Si no retorna ninguna fila, la peticion no existe
        if (sqlite3_step(sqlStatement) != SQLITE_ROW) {
            sqlite3_finalize(sqlStatement);
            queryResult.code = CODE_PETITION_NOT_FOUND;
            queryResult.message = "Peticion con ID " + to_string(id) + " no encontrada";
            return queryResult;
        }

        // Mapear las columnas al struct de salida
        Petition petitionRow;
        petitionRow.id = sqlite3_column_int(sqlStatement, 0);
        petitionRow.requesterName = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 1));
        petitionRow.type = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 2));
        petitionRow.description = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 3));
        petitionRow.response = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 4));
        petitionRow.status = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 5));
        petitionRow.queuePosition = sqlite3_column_int(sqlStatement, 6);

        sqlite3_finalize(sqlStatement);

        queryResult.data.push_back(petitionRow);
        queryResult.code = CODE_PETITION_LISTED;
        queryResult.message = "Peticion encontrada";
        return queryResult;
    }
    catch (exception& e) {
        cout << "DB Exception: " << string(e.what());
        queryResult.code = CODE_EXCEPTION;
        queryResult.message = "Excepcion no esperada en obtainPetitionById";
        return queryResult;
    }
    catch (...) {
        queryResult.code = CODE_EXCEPTION;
        queryResult.message = "Excepcion desconocida en obtainPetitionById";
        return queryResult;
    }
}

DBQueryResponse<Petition> Connection::obtainNextPetition() {
    DBQueryResponse<Petition> queryResult;
    try {
        // Obtener la peticion pendiente con menor QUEUE_POSITION (frente de la cola FIFO)
        const char* sqlQuery =
            "SELECT ID, REQUESTER_NAME, TYPE, DESCRIPTION, RESPONSE, STATUS, QUEUE_POSITION "
            "FROM TB_PETITION WHERE STATUS = 'Pendiente' ORDER BY QUEUE_POSITION LIMIT 1;";
        sqlite3_stmt* sqlStatement = nullptr;

        // Compilar la query
        if (sqlite3_prepare_v2(db_, sqlQuery, -1, &sqlStatement, nullptr) != SQLITE_OK) {
            DbResponse errorResponse = sqliteError(CODE_ERROR_DB, "obtainNextPetition::prepare");
            queryResult.code = errorResponse.code;
            queryResult.message = errorResponse.message;
            return queryResult;
        }

        // Si no retorna ninguna fila, no hay peticiones pendientes
        if (sqlite3_step(sqlStatement) != SQLITE_ROW) {
            sqlite3_finalize(sqlStatement);
            queryResult.code = CODE_PETITION_NOT_FOUND;
            queryResult.message = "No hay peticiones pendientes";
            return queryResult;
        }

        // Mapear las columnas al struct de salida
        Petition petitionRow;
        petitionRow.id = sqlite3_column_int(sqlStatement, 0);
        petitionRow.requesterName = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 1));
        petitionRow.type = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 2));
        petitionRow.description = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 3));
        petitionRow.status = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 4));
        petitionRow.queuePosition = sqlite3_column_int(sqlStatement, 5);

        sqlite3_finalize(sqlStatement);
        queryResult.data.push_back(petitionRow);
        queryResult.code = CODE_PETITION_LISTED;
        queryResult.message = "Siguiente peticion obtenida";
        return queryResult;
    }
    catch (exception& e) {
        cout << "DB Exception: " << string(e.what());
        queryResult.code = CODE_EXCEPTION;
        queryResult.message = "Excepcion no esperada en obtainNextPetition";
        return queryResult;
    }
    catch (...) {
        queryResult.code = CODE_EXCEPTION;
        queryResult.message = "Excepcion desconocida en obtainNextPetition";
        return queryResult;
    }
}

DbResponse Connection::updatePetitionStatus(int id, string response, string status) {
    try {
        // Validacion: el nombre del solicitante no puede estar vacio
        if (response.empty()) {
            return { -1, CODE_PETITION_INVALID_DATA, "La respuesta no puede estar vacio" };
        }

        // Validacion: el nuevo estado debe ser Atendida o Cancelada
        if (status != "Atendida" && status != "Cancelada") {
            return { -1, CODE_PETITION_INVALID_DATA, "Estado invalido. Use: Atendida o Cancelada" };
        }

        // Validacion: la peticion debe existir y estar en estado Pendiente
        // No se puede cambiar el estado de una peticion ya procesada
        string checkPendingQuery =
            "SELECT COUNT(*) FROM TB_PETITION WHERE ID = " + to_string(id) + " AND STATUS = 'Pendiente';";
        if (!rowExists(db_, checkPendingQuery)) {
            return { -1, CODE_PETITION_NOT_FOUND, "Peticion no encontrada o ya procesada (ID: " + to_string(id) + ")" };
        }

        // Actualizar el estado de la peticion
        const char* sqlQuery = "UPDATE TB_PETITION SET RESPONSE = ?, STATUS = ? WHERE ID = ?;";
        sqlite3_stmt* sqlStatement = nullptr;

        // Compilar la query
        if (sqlite3_prepare_v2(db_, sqlQuery, -1, &sqlStatement, nullptr) != SQLITE_OK) {
            return sqliteError(CODE_ERROR_DB, "updatePetitionStatus::prepare");
        }

        // Enlazar parametros: primer ? = status, segundo ? = id
        sqlite3_bind_text(sqlStatement, 1, response.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(sqlStatement, 2, status.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(sqlStatement, 3, id);

        int resultCode = sqlite3_step(sqlStatement);
        sqlite3_finalize(sqlStatement);

        if (resultCode != SQLITE_DONE) {
            return sqliteError(CODE_ERROR_DB, "updatePetitionStatus::step");
        }

        return { id, CODE_PETITION_ATTENDED, "Peticion actualizada a: " + status };
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
        // Validacion: verificar que la peticion exista antes de eliminarla
        string checkExistQuery = "SELECT COUNT(*) FROM TB_PETITION WHERE ID = " + to_string(id) + ";";
        if (!rowExists(db_, checkExistQuery)) {
            return { -1, CODE_PETITION_NOT_FOUND, "Peticion con ID " + to_string(id) + " no encontrada" };
        }

        // Eliminar la peticion por su ID
        const char* sqlQuery = "DELETE FROM TB_PETITION WHERE ID = ?;";
        sqlite3_stmt* sqlStatement = nullptr;

        // Compilar la query
        if (sqlite3_prepare_v2(db_, sqlQuery, -1, &sqlStatement, nullptr) != SQLITE_OK) {
            return sqliteError(CODE_ERROR_DB, "deletePetition::prepare");
        }

        // Reemplazar el ? con el ID de la peticion a eliminar
        sqlite3_bind_int(sqlStatement, 1, id);

        int resultCode = sqlite3_step(sqlStatement);
        sqlite3_finalize(sqlStatement);

        if (resultCode != SQLITE_DONE) {
            return sqliteError(CODE_ERROR_DB, "deletePetition::step");
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

DbResponse Connection::insertHistoric(string actionType, string entityType, int recordId, string previousData, string newData, int stackPosition)
{
    try {
        // Validaciones básicas
        if (actionType.empty()) {
            return { -1, CODE_HISTORIC_INVALID_DATA, "El tipo de accion no puede estar vacio" };
        }

        if (entityType.empty()) {
            return { -1, CODE_HISTORIC_INVALID_DATA, "El tipo de entidad no puede estar vacio" };
        }

        if (previousData.empty()) {
            return { -1, CODE_HISTORIC_INVALID_DATA, "Los datos previos son necesarios para permitir deshacer" };
        }

        const char* sqlQuery =
            "INSERT INTO TB_HISTORIC "
            "(ACTION_TYPE, ENTITY_TYPE, RECORD_ID, PREVIOUS_DATA, NEW_DATA, STACK_POSITION) "
            "VALUES (?, ?, ?, ?, ?, ?);";

        sqlite3_stmt* sqlStatement = nullptr;

        if (sqlite3_prepare_v2(db_, sqlQuery, -1, &sqlStatement, nullptr) != SQLITE_OK) {
            return sqliteError(CODE_ERROR_DB, "insertHistoric::prepare");
        }

        sqlite3_bind_text(sqlStatement, 1, actionType.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(sqlStatement, 2, entityType.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(sqlStatement, 3, recordId);
        sqlite3_bind_text(sqlStatement, 4, previousData.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(sqlStatement, 5, newData.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(sqlStatement, 6, stackPosition);

        int resultCode = sqlite3_step(sqlStatement);
        sqlite3_finalize(sqlStatement);

        if (resultCode != SQLITE_DONE) {
            return sqliteError(CODE_ERROR_DB, "insertHistoric::step");
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
    DBQueryResponse<Historic> queryResult;

    try {
        queryResult.data.clear();

        const char* sqlQuery =
            "SELECT ID, ACTION_TYPE, ENTITY_TYPE, RECORD_ID, PREVIOUS_DATA, NEW_DATA, STACK_POSITION "
            "FROM TB_HISTORIC "
            "ORDER BY STACK_POSITION ASC;";

        sqlite3_stmt* sqlStatement = nullptr;

        if (sqlite3_prepare_v2(db_, sqlQuery, -1, &sqlStatement, nullptr) != SQLITE_OK) {
            DbResponse errorResponse = sqliteError(CODE_ERROR_DB, "listHistoric::prepare");
            queryResult.code = errorResponse.code;
            queryResult.message = errorResponse.message;
            return queryResult;
        }

        while (sqlite3_step(sqlStatement) == SQLITE_ROW) {
            Historic historicRow;

            historicRow.id = sqlite3_column_int(sqlStatement, 0);
            historicRow.actionType = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 1));
            historicRow.entityType = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 2));
            historicRow.recordId = sqlite3_column_int(sqlStatement, 3);
            historicRow.previousData = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 4));
            historicRow.newData = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 5));
            historicRow.stackPosition = sqlite3_column_int(sqlStatement, 6);

            queryResult.data.push_back(historicRow);
        }

        sqlite3_finalize(sqlStatement);

        queryResult.code = CODE_HISTORIC_LISTED;

        if (queryResult.data.empty()) {
            queryResult.message = "No hay acciones en el historial";
        }
        else {
            queryResult.message = "Historial obtenido: " + to_string(queryResult.data.size()) + " entradas";
        }

        return queryResult;
    }
    catch (exception& e) {
        cout << "DB Exception: " << string(e.what());
        queryResult.code = CODE_EXCEPTION;
        queryResult.message = "Excepcion no esperada en listHistoric";
        return queryResult;
    }
    catch (...) {
        queryResult.code = CODE_EXCEPTION;
        queryResult.message = "Excepcion desconocida en listHistoric";
        return queryResult;
    }
}

DbResponse Connection::deleteLastHistoric() {
    try {
        const char* sqlQuerySelect =
            "SELECT ID FROM TB_HISTORIC "
            "ORDER BY STACK_POSITION DESC LIMIT 1;";

        sqlite3_stmt* sqlStatementSelect = nullptr;

        if (sqlite3_prepare_v2(db_, sqlQuerySelect, -1, &sqlStatementSelect, nullptr) != SQLITE_OK) {
            return sqliteError(CODE_ERROR_DB, "deleteLastHistoric::select::prepare");
        }

        if (sqlite3_step(sqlStatementSelect) != SQLITE_ROW) {
            sqlite3_finalize(sqlStatementSelect);
            return { -1, CODE_HISTORIC_NOT_FOUND, "No hay acciones para deshacer" };
        }

        int lastHistoricId = sqlite3_column_int(sqlStatementSelect, 0);
        sqlite3_finalize(sqlStatementSelect);

        const char* sqlQueryDelete = "DELETE FROM TB_HISTORIC WHERE ID = ?;";
        sqlite3_stmt* sqlStatementDelete = nullptr;

        if (sqlite3_prepare_v2(db_, sqlQueryDelete, -1, &sqlStatementDelete, nullptr) != SQLITE_OK) {
            return sqliteError(CODE_ERROR_DB, "deleteLastHistoric::delete::prepare");
        }

        sqlite3_bind_int(sqlStatementDelete, 1, lastHistoricId);

        int resultCode = sqlite3_step(sqlStatementDelete);
        sqlite3_finalize(sqlStatementDelete);

        if (resultCode != SQLITE_DONE) {
            return sqliteError(CODE_ERROR_DB, "deleteLastHistoric::delete::step");
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
