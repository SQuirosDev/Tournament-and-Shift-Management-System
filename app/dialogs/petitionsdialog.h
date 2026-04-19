#pragma once

#include <QtWidgets/QDialog>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLabel>
#include "logPetition.h"

class petitionsDialog : public QDialog {
Q_OBJECT
public:
explicit petitionsDialog(LogPetition* logPetition, QWidget* parent = nullptr);
~petitionsDialog();

private:
LogPetition* logPetition_;

/*
// Pestaña "Ver peticiones"
QListWidget*  listPetitions_;
QLabel*       lblSubtitle_;
QLabel*       lblNextInQueue_;
QPushButton*  btnRespond_;
QPushButton*  btnDelete_;

// Pestaña "Nueva petición"
QLineEdit*    edtRequester_;
QComboBox*    cmbType_;
QTextEdit*    edtDescription_;

void loadPetitions();
void updateNextInQueue();
*/

private slots:
void onAddClicked();
void onRespondClicked();
void onDeleteClicked();
void onRefresh();
};