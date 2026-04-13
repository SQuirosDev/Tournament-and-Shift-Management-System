#pragma once

#include <QtWidgets/QDialog>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLabel>
#include "logPlayer.h"
#include "logTeam.h"

class playersDialog : public QDialog {
    Q_OBJECT
public:
    explicit playersDialog(LogPlayer* logPlayer, LogTeam* logTeam,
        QWidget* parent = nullptr);
    ~playersDialog();

private:
    LogPlayer* logPlayer_;
    LogTeam* logTeam_;

    QListWidget* listPlayers_;
    QLineEdit* edtName_;
    QComboBox* cmbTeam_;
    QComboBox* cmbFilter_;
    QPushButton* btnEdit_;
    QPushButton* btnDelete_;
    QLabel* lblSubtitle_;

    void loadTeams();
    void loadPlayers();

private slots:
    void onAddClicked();
    void onRefresh();
    void onEditClicked();
    void onDeleteClicked();
};
