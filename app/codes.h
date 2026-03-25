#pragma once

// ============================================================
//  Prefijos:
//    1xx  - Exito general
//    2xx  - Exito por modulo
//    4xx  - Error de validacion / cliente
//    5xx  - Error interno / base de datos
// ============================================================

// ----------------------------
//  GENERAL (1xx)
// ----------------------------
#define CODE_OK                         100
#define CODE_SELECT_OK                  101
#define CODE_INSERT_OK                  102
#define CODE_UPDATE_OK                  103
#define CODE_DELETE_OK                  104

// ----------------------------
//  TOURNAMENT (2xx)
// ----------------------------
#define CODE_TOURNAMENT_CREATED         200
#define CODE_TOURNAMENT_LISTED          201
#define CODE_TOURNAMENT_UPDATED         202
#define CODE_TOURNAMENT_NOT_FOUND       400
#define CODE_TOURNAMENT_DUPLICATE       401
#define CODE_TOURNAMENT_INVALID_DATA    402

// ----------------------------
//  TEAM (21x)
// ----------------------------
#define CODE_TEAM_CREATED               210
#define CODE_TEAM_LISTED                211
#define CODE_TEAM_UPDATED               212
#define CODE_TEAM_DELETED               213
#define CODE_TEAM_NOT_FOUND             410
#define CODE_TEAM_DUPLICATE             411
#define CODE_TEAM_INVALID_DATA          412

// ----------------------------
//  PLAYER (22x)
// ----------------------------
#define CODE_PLAYER_CREATED             220
#define CODE_PLAYER_LISTED              221
#define CODE_PLAYER_UPDATED             222
#define CODE_PLAYER_DELETED             223
#define CODE_PLAYER_NOT_FOUND           420
#define CODE_PLAYER_DUPLICATE           421
#define CODE_PLAYER_INVALID_DATA        422

// ----------------------------
//  MATCH (23x)
// ----------------------------
#define CODE_MATCH_CREATED              230
#define CODE_MATCH_LISTED               231
#define CODE_MATCH_UPDATED              232
#define CODE_MATCH_DELETED              233
#define CODE_MATCH_NOT_FOUND            430
#define CODE_MATCH_INVALID_DATA         431

// ----------------------------
//  PETITION (24x)
// ----------------------------
#define CODE_PETITION_CREATED           240
#define CODE_PETITION_LISTED            241
#define CODE_PETITION_ATTENDED          242
#define CODE_PETITION_NOT_FOUND         440
#define CODE_PETITION_INVALID_DATA      441

// ----------------------------
//  HISTORIC (25x)
// ----------------------------
#define CODE_HISTORIC_LISTED            250
#define CODE_HISTORIC_DELETED           251
#define CODE_HISTORIC_NOT_FOUND         450
#define CODE_HISTORIC_INVALID           451

// ----------------------------
//  DATABASE / INTERNAL (5xx)
// ----------------------------
#define CODE_DB_CONNECTION_ERROR        500
#define CODE_DB_INIT_ERROR              501
#define CODE_DB_QUERY_ERROR             502
#define CODE_DB_PREPARE_ERROR           503
#define CODE_DB_STEP_ERROR              504
#define CODE_DB_BIND_ERROR              505
#define CODE_EXCEPTION                  599