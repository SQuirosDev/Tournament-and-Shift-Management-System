#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_app.h"
#include "connection.h"
#include <QtWidgets/QLabel>

class app : public QMainWindow
{
    Q_OBJECT

public:
    app(QWidget *parent = nullptr);
    ~app();

private:
    Ui::appClass ui;
    Connection* conn_;
    QLabel* lblWelcome;
    void updateUiState();
private slots:
    void onPlayersClicked();
    void onTeamsClicked();
    void onTournamentsClicked();
    void onMatchesClicked();
};

