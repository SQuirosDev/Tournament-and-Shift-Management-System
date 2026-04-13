#pragma once
#include <QtWidgets/QDialog>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLabel>
#include <functional>
#include "logMatch.h"
#include "logGame.h"
#include "logTournament.h"
#include "logTeam.h"
#include "BackendResponse.h"

class matchesDialog : public QDialog {
    Q_OBJECT
public:
    explicit matchesDialog(LogMatch* logMatch, LogGame* logGame,
        LogTournament* logTournament, LogTeam* logTeam,
        QWidget* parent = nullptr);
    ~matchesDialog();

private:
    LogMatch* logMatch_;
    LogGame* logGame_;
    LogTournament* logTournament_;
    LogTeam* logTeam_;

    QListWidget* listMatches_;
    QTableWidget* tableStandings_;
    QComboBox* cmbTournament_;
    QComboBox* cmbTeamA_;
    QComboBox* cmbTeamB_;
    QPushButton* btnMarkPlayed_;
    QPushButton* btnEditResult_;
    QPushButton* btnDeleteMatch_;
    QLabel* lblSubtitle_;

    QPushButton* btnGenGroups_;
    QPushButton* btnPlayGroups_;
    QPushButton* btnGenSemi_;
    QPushButton* btnPlaySemi_;
    QPushButton* btnGenFinal_;
    QPushButton* btnPlayFinal_;

    void loadTournaments();
    void loadTeamsForTournament(int tournamentId);
    void loadMatches(int tournamentId);
    void loadStandings(int tournamentId);

private slots:
    void onAddClicked();
    void onEditClicked();
    void onDeleteClicked();
    void onRefresh();
};
