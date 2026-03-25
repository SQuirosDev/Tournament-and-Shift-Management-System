#pragma once

#include <string>

using namespace std;

struct Team {
    int    id = 0;
    int    tournamentId = 0;
    string name;
    int    tournaments = 0;
    int    points = 0;
    int    wins = 0;
    int    draws = 0;
    int    losses = 0;
};