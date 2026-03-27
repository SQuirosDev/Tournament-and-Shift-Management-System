#pragma once

#include <string>

using namespace std;

struct Tournament {
    int id = 0;
    string name;
    string phase; // "Registro" | "Grupos" | "Eliminacion" | "Finalizado"
};