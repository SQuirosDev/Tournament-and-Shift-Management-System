#pragma once

#include <QtWidgets/QDialog>
#include "connection.h"
#include "dialogs/ui_teamsdialog.h"

class teamsDialog : public QDialog {
public:
    teamsDialog(Connection* conn, QWidget* parent = nullptr);
    ~teamsDialog();

private slots:
    void onAddClicked();
    void onRefresh();
    void onEditClicked();
    void onDeleteClicked();

private:
    Connection* conn_;
    Ui::teamsDialog ui;
    void loadTournaments();
    void loadTeams();
};
