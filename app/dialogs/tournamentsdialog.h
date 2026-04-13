#pragma once
#include <QtWidgets/QDialog>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLabel>
#include "logTournament.h"

class tournamentsDialog : public QDialog {
    Q_OBJECT
public:
    explicit tournamentsDialog(LogTournament* logTournament, QWidget* parent = nullptr);
    ~tournamentsDialog();

private:
    LogTournament* logTournament_;

    // Widgets
    QListWidget* listTournaments_;
    QLineEdit* edtName_;
    QPushButton* btnEdit_;
    QPushButton* btnDelete_;
    QComboBox* cmbPhase_;
    QWidget* phaseRow_;
    QLabel* lblSubtitle_;

    void loadTournaments();
    void ensureUi();

private slots:
    void onAddClicked();
    void onRefresh();
    void onEditClicked();
    void onDeleteClicked();
    void onChangePhaseClicked();
};
