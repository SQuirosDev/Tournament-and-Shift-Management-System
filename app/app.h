#pragma once
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QLabel>
#include "ui_app.h"
#include "connection.h"
#include "logTournament.h"
#include "logTeam.h"
#include "logPlayer.h"
#include "logMatch.h"
#include "logGame.h"
#include "logHistoric.h"
#include "logPetition.h"

class app : public QMainWindow
{
    Q_OBJECT

public:
    app(QWidget* parent = nullptr);
    ~app();

private:
    Ui::appClass ui;

    // DB
    Connection* conn_;

    // Capa lógica
    LogHistoric* logHistoric_;
    LogTournament* logTournament_;
    LogTeam* logTeam_;
    LogPlayer* logPlayer_;
    LogMatch* logMatch_;
    LogGame* logGame_;
	LogPetition* logPetition_;

    // UI
    QLabel* lblWelcome;
    QPushButton* btnUndo_ = nullptr;

    //Voids
    void onUndoClicked();
    void updateUiState();

private slots:
    void onPlayersClicked();
    void onTeamsClicked();
    void onTournamentsClicked();
    void onMatchesClicked();
};
