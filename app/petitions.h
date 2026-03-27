#pragma once

#include <string>

using namespace std;

struct Petition {
    int id = 0;
    string requesterName;
    string type;          // "Inscripcion" | "Consulta" | "Apelacion"
    string description;
    string status;        // "Pendiente" | "Atendida" | "Cancelada"
    int queuePosition = 0;
};