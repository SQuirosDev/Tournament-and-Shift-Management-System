#pragma once

#include <string>

using namespace std;

struct Historic {
    int    id = 0;
    int    tournamentId = 0;
    string actionType;    // "Crear Torneo" | "Registrar Jugador" | "Generar Equipos" | "Ingresar Resultado" | "Generar Bracket" | "Atender Solicitud"
    string previousData; // JSON con estado anterior (para revertir)
    string newData;      // JSON con estado nuevo
    int    stackPosition = 0;
};