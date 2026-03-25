-- =============================================================================
-- PRAGMA: Para que SQLite tome enserio todas las foreign key
-- =============================================================================
PRAGMA FOREIGN_KEYS = ON;

-- =============================================================================
-- TABLA: TOURNAMENT
-- =============================================================================
CREATE TABLE IF NOT EXISTS TB_TOURNAMENT (
    ID INTEGER PRIMARY KEY AUTOINCREMENT,
    NAME TEXT NOT NULL CHECK(NAME != ''),
    PHASE TEXT NOT NULL DEFAULT 'Registro' CHECK(PHASE IN ('Registro', 'Grupos', 'Eliminacion', 'Finalizado'))
);

-- =============================================================================
-- TABLA: TEAM
-- =============================================================================
CREATE TABLE IF NOT EXISTS TB_TEAM (
    ID INTEGER PRIMARY KEY AUTOINCREMENT,
    TOURNAMENT_ID INTEGER NOT NULL REFERENCES TB_TOURNAMENT(ID) ON DELETE CASCADE,
    NAME TEXT NOT NULL CHECK(NAME != ''),
    TOURNAMENTS INTEGER NOT NULL DEFAULT 0 CHECK(TOURNAMENTS >= 0),
    POINTS INTEGER NOT NULL DEFAULT 0 CHECK(POINTS >= 0),
    WINS INTEGER NOT NULL DEFAULT 0 CHECK(WINS >= 0),
    DRAWS INTEGER NOT NULL DEFAULT 0 CHECK(DRAWS >= 0),
    LOSSES INTEGER NOT NULL DEFAULT 0 CHECK(LOSSES >= 0)
);

-- =============================================================================
-- TABLA: PLAYER
-- =============================================================================
CREATE TABLE IF NOT EXISTS TB_PLAYER (
    ID INTEGER PRIMARY KEY AUTOINCREMENT,
    TEAM_ID INTEGER NOT NULL REFERENCES TB_TEAM(ID) ON DELETE CASCADE,
    NAME TEXT NOT NULL CHECK(NAME != '')
);

-- =============================================================================
-- TABLA: MATCH
-- =============================================================================
CREATE TABLE IF NOT EXISTS TB_MATCH (
    ID INTEGER PRIMARY KEY AUTOINCREMENT,
    TOURNAMENT_ID INTEGER NOT NULL REFERENCES TB_TOURNAMENT(ID) ON DELETE CASCADE,
    TEAM_A_ID INTEGER NOT NULL REFERENCES TB_TEAM(ID),
    TEAM_B_ID INTEGER NOT NULL REFERENCES TB_TEAM(ID),
    PHASE TEXT NOT NULL CHECK(PHASE IN ('Grupos', 'Eliminacion')),
    ROUND INTEGER NOT NULL DEFAULT 1 CHECK(ROUND >= 1),
    STATUS TEXT NOT NULL DEFAULT 'Pendiente' CHECK(STATUS IN ('Pendiente', 'Finalizado')),
    WINNER_ID INTEGER REFERENCES TB_TEAM(ID),
    RESULT TEXT CHECK(RESULT IN ('Gana A', 'Gana B', 'Empate', NULL)),
    QUEUE_POSITION INTEGER NOT NULL DEFAULT 0,
    PLAYED_AT TEXT,
    CHECK(TEAM_A_ID != TEAM_B_ID)
);

-- =============================================================================
-- TABLA: PETITION
-- =============================================================================
CREATE TABLE IF NOT EXISTS TB_PETITION (
    ID INTEGER PRIMARY KEY AUTOINCREMENT,
    REQUESTER_NAME TEXT NOT NULL CHECK(REQUESTER_NAME != ''),
    TYPE TEXT NOT NULL CHECK(TYPE IN ('Inscripcion', 'Consulta', 'Apelacion')),
    DESCRIPTION TEXT NOT NULL DEFAULT '',
    STATUS TEXT NOT NULL DEFAULT 'Pendiente' CHECK(STATUS IN ('Pendiente', 'Atendida', 'Cancelada')),
    QUEUE_POSITION INTEGER NOT NULL DEFAULT 0
);

-- =============================================================================
-- TABLA: ACTION
-- =============================================================================
CREATE TABLE IF NOT EXISTS TB_ACTION (
    ID INTEGER PRIMARY KEY AUTOINCREMENT,
    TOURNAMENT_ID INTEGER REFERENCES TB_TOURNAMENT(ID) ON DELETE CASCADE,
    ACTION_TYPE TEXT NOT NULL CHECK(ACTION_TYPE IN ('Crear Torneo', 'Registrar Jugador', 'Generar Equipos', 'Ingresar Resultado', 'Generar Bracket', 'Atender Solicitud')),
    PAYLOAD TEXT NOT NULL DEFAULT '{}',
    IS_REVERTED INTEGER NOT NULL DEFAULT 0 CHECK(IS_REVERTED IN (0, 1)),
    STACK_POSITION INTEGER NOT NULL DEFAULT 0
);

-- =============================================================================
-- ÍNDICES — mejoran consultas frecuentes sin afectar la lógica de pilas/colas
-- =============================================================================

CREATE INDEX IF NOT EXISTS IDX_TEAM_TOURNAMENT ON TB_TEAM(TOURNAMENT_ID);
CREATE INDEX IF NOT EXISTS IDX_PLAYER_TEAM ON TB_PLAYER(TEAM_ID);
CREATE INDEX IF NOT EXISTS IDX_MATCH_TOURNAMENT ON TB_MATCH(TOURNAMENT_ID, QUEUE_POSITION);
CREATE INDEX IF NOT EXISTS IDX_MATCH_STATUS ON TB_MATCH(TOURNAMENT_ID, PHASE, STATUS);
CREATE INDEX IF NOT EXISTS IDX_PETITION_QUEUE ON TB_PETITION(STATUS, QUEUE_POSITION);
CREATE INDEX IF NOT EXISTS IDX_ACTION_STACK ON TB_ACTION(TOURNAMENT_ID, STACK_POSITION DESC);
