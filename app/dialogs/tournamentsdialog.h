#pragma once

#include <QtWidgets/QDialog>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include "connection.h"
#include "dialogs/ui_tournamentsdialog.h"
#include "dialogs/dialog_manager.h"

class tournamentsDialog : public QDialog {
public:
    tournamentsDialog(Connection* conn, QWidget* parent = nullptr);
    ~tournamentsDialog();

private:
    Connection* conn_;
    Ui::tournamentsDialog ui;
    void loadTournaments();
    void ensureUi();
    void onAddClicked();
    void onRefresh();
    void onEditClicked();
    void onDeleteClicked();
};
