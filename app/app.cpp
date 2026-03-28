#include "app.h"
#include <QtWidgets/QMessageBox>

app::app(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    // Connect buttons to slots
    connect(ui.btnPlayers, &QPushButton::clicked, this, &app::onPlayersClicked);
    connect(ui.btnTeams, &QPushButton::clicked, this, &app::onTeamsClicked);
    connect(ui.btnTournaments, &QPushButton::clicked, this, &app::onTournamentsClicked);
    connect(ui.btnMatches, &QPushButton::clicked, this, &app::onMatchesClicked);
}

app::~app()
{}

void app::onPlayersClicked()
{
    QMessageBox::information(this, "Players", "Open Players form (not implemented yet)");
}

void app::onTeamsClicked()
{
    QMessageBox::information(this, "Teams", "Open Teams form (not implemented yet)");
}

void app::onTournamentsClicked()
{
    QMessageBox::information(this, "Tournaments", "Open Tournaments form (not implemented yet)");
}

void app::onMatchesClicked()
{
    QMessageBox::information(this, "Matches", "Open Matches form (not implemented yet)");
}

