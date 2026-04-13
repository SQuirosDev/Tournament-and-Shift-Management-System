#pragma once
#include <QtWidgets/QDialog>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLabel>
#include "logTeam.h"
#include "logTournament.h"

class teamsDialog : public QDialog {
    Q_OBJECT
public:
    explicit teamsDialog(LogTeam* logTeam, LogTournament* logTournament,
        QWidget* parent = nullptr);
    ~teamsDialog();

private:
    LogTeam* logTeam_;
    LogTournament* logTournament_;

    QListWidget* listTeams_;
    QLineEdit* edtName_;
    QComboBox* cmbTournament_;
    QPushButton* btnEdit_;
    QPushButton* btnDelete_;
    QLabel* lblSubtitle_;

    void loadTournaments();
    void loadTeams();

private slots:
    void onAddClicked();
    void onRefresh();
    void onEditClicked();
    void onDeleteClicked();
};
