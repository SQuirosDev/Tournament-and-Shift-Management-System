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
    ROUND INTEGER NOT NULL DEFAULT 1 CHECK(ROUND >= 0),
    STATUS TEXT NOT NULL DEFAULT 'Pendiente' CHECK(STATUS IN ('Pendiente', 'Finalizado')),
    WINNER_ID INTEGER REFERENCES TB_TEAM(ID),
    RESULT TEXT CHECK(RESULT IN ('Gana A', 'Gana B', 'Empate', NULL)),
    QUEUE_POSITION INTEGER NOT NULL DEFAULT 0,
    CHECK(TEAM_A_ID != TEAM_B_ID)
);

-- =============================================================================
-- TABLA: PETITION
-- =============================================================================
CREATE TABLE IF NOT EXISTS TB_PETITION (
    ID INTEGER PRIMARY KEY AUTOINCREMENT,
    REQUESTER_NAME TEXT NOT NULL CHECK(REQUESTER_NAME != ''),
    TYPE TEXT NOT NULL CHECK(TYPE IN ('Inscripcion', 'Consulta', 'Apelacion')),
    DESCRIPTION TEXT NOT NULL,
    RESPONSE TEXT NOT NULL DEFAULT '',
    STATUS TEXT NOT NULL DEFAULT 'Pendiente' CHECK(STATUS IN ('Pendiente', 'Atendida', 'Cancelada')),
    QUEUE_POSITION INTEGER NOT NULL DEFAULT 0
);

-- =============================================================================
-- TABLA: HISTORIC
-- =============================================================================
CREATE TABLE IF NOT EXISTS TB_HISTORIC (
    ID INTEGER PRIMARY KEY AUTOINCREMENT,
    ACTION_TYPE TEXT NOT NULL,
    ENTITY_TYPE TEXT NOT NULL,
    RECORD_ID INTEGER,
    PREVIOUS_DATA TEXT NOT NULL DEFAULT '{}',
    NEW_DATA TEXT NOT NULL DEFAULT '{}',
    STACK_POSITION INTEGER NOT NULL DEFAULT 0,
);

-- =============================================================================
-- ÍNDICES — mejoran consultas frecuentes sin afectar la lógica de pilas/colas
-- =============================================================================

CREATE INDEX IF NOT EXISTS IDX_TEAM_TOURNAMENT ON TB_TEAM(TOURNAMENT_ID);
CREATE INDEX IF NOT EXISTS IDX_PLAYER_TEAM ON TB_PLAYER(TEAM_ID);
CREATE INDEX IF NOT EXISTS IDX_MATCH_TOURNAMENT ON TB_MATCH(TOURNAMENT_ID, QUEUE_POSITION);
CREATE INDEX IF NOT EXISTS IDX_MATCH_STATUS ON TB_MATCH(TOURNAMENT_ID, PHASE, STATUS);
CREATE INDEX IF NOT EXISTS IDX_PETITION_QUEUE ON TB_PETITION(STATUS, QUEUE_POSITION);
CREATE INDEX IF NOT EXISTS IDX_HISTORIC_STACK ON TB_HISTORIC(TOURNAMENT_ID, STACK_POSITION DESC);
