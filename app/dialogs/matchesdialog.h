#pragma once

#include <QtWidgets/QDialog>
#include "connection.h"
#include "dialogs/ui_matchesdialog.h"

class matchesDialog : public QDialog {
public:
    matchesDialog(Connection* conn, QWidget* parent = nullptr);
    ~matchesDialog();

private slots:
    void onAddClicked();
    void onEditClicked();
    void onDeleteClicked();
    void onRefresh();

private:
    Connection* conn_;
    Ui::matchesDialog ui;
    void loadTournaments();
    void loadTeamsForTournament(int tournamentId);
    void loadMatches(int tournamentId);
};
