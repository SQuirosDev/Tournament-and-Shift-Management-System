app/
│
├───main.cpp
├───torneos.db
│
├───data/ # Pila y Cola Generica (Nose para que seran)
│   ├───stack.h ?
│   └───queue.h ?
│
├───models/ # Estructuras
│   ├───tournaments.h ✅
│   ├───teams.h ✅
│   ├───players.h ✅
│   ├───matches.h ✅
│   ├───petitions.h ✅
│   └───historic.h ✅
│
├───logics/
│   ├───logTournament.h
│   ├───logTournament.cpp
│   ├───logTeam.h
│   ├───logTeam.cpp
│   ├───logPlayer.h ✅
│   ├───logPlayer.cpp ✅
│   ├───logMatch.h
│   ├───logMatch.cpp
│   ├───logPetition.h
│   ├───logPetition.cpp
│   ├───logHistory.h
│   └───logHistory.cpp
│
├───dataAccess/
│   ├───connection.h ✅
│   └───connection.cpp ✅
│
├───views/
│   ├───
│   └───