#pragma once

#include <QtCore/QVariant>
#include <QtCore/QCoreApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QComboBox>

QT_BEGIN_NAMESPACE

class Ui_matchesDialog
{
public:
    QWidget *verticalLayoutWidget;
    QVBoxLayout *verticalLayout;
    QListWidget *listMatches;
    QHBoxLayout *horizontalLayout;
    QComboBox *cmbTournament;
    QComboBox *cmbTeamA;
    QComboBox *cmbTeamB;
    QPushButton *btnAdd;
    QPushButton *btnRefresh;

    void setupUi(QDialog *matchesDialog)
    {
        if (matchesDialog->objectName().isEmpty())
            matchesDialog->setObjectName("matchesDialog");
        matchesDialog->resize(500, 350);
        verticalLayoutWidget = new QWidget(matchesDialog);
        verticalLayoutWidget->setObjectName("verticalLayoutWidget");
        verticalLayoutWidget->setGeometry(QRect(10, 10, 481, 331));
        verticalLayout = new QVBoxLayout(verticalLayoutWidget);
        verticalLayout->setObjectName("verticalLayout");
        listMatches = new QListWidget(verticalLayoutWidget);
        listMatches->setObjectName("listMatches");
        verticalLayout->addWidget(listMatches);
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        cmbTournament = new QComboBox(verticalLayoutWidget);
        cmbTournament->setObjectName("cmbTournament");
        horizontalLayout->addWidget(cmbTournament);
        cmbTeamA = new QComboBox(verticalLayoutWidget);
        cmbTeamA->setObjectName("cmbTeamA");
        horizontalLayout->addWidget(cmbTeamA);
        cmbTeamB = new QComboBox(verticalLayoutWidget);
        cmbTeamB->setObjectName("cmbTeamB");
        horizontalLayout->addWidget(cmbTeamB);
        btnAdd = new QPushButton(verticalLayoutWidget);
        btnAdd->setObjectName("btnAdd");
        btnAdd->setText("Add");
        horizontalLayout->addWidget(btnAdd);
        verticalLayout->addLayout(horizontalLayout);
        btnRefresh = new QPushButton(verticalLayoutWidget);
        btnRefresh->setObjectName("btnRefresh");
        btnRefresh->setText("Refresh");
        verticalLayout->addWidget(btnRefresh);

        retranslateUi(matchesDialog);

        QMetaObject::connectSlotsByName(matchesDialog);
    }

    void retranslateUi(QDialog *matchesDialog)
    {
        matchesDialog->setWindowTitle(QCoreApplication::translate("matchesDialog", "Matches", nullptr));
    }

};

namespace Ui { class matchesDialog: public Ui_matchesDialog {}; }

QT_END_NAMESPACE
