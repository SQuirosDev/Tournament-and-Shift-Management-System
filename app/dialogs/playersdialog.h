#pragma once

#include <QtWidgets/QDialog>
#include "connection.h"
#include "dialogs/ui_playersdialog.h"

class playersDialog : public QDialog {
public:
    playersDialog(Connection* conn, QWidget* parent = nullptr);
    ~playersDialog();

private slots:
    void onAddClicked();
    void onRefresh();
    void onEditClicked();
    void onDeleteClicked();

private:
    Connection* conn_;
    Ui::playersDialog ui;
    void loadPlayers();
    void loadTeams();
};
