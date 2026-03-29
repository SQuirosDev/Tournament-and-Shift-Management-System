#pragma once

#include <QtCore/QVariant>
#include <QtCore/QCoreApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QComboBox>

QT_BEGIN_NAMESPACE

class Ui_teamsDialog
{
public:
    QWidget *verticalLayoutWidget;
    QVBoxLayout *verticalLayout;
    QListWidget *listTeams;
    QHBoxLayout *horizontalLayout;
    QLineEdit *edtName;
    QComboBox *cmbTournament;
    QPushButton *btnAdd;
    QPushButton *btnRefresh;
    QHBoxLayout *horizontalLayout2;
    QPushButton *btnEdit;
    QPushButton *btnDelete;

    void setupUi(QDialog *teamsDialog)
    {
        if (teamsDialog->objectName().isEmpty())
            teamsDialog->setObjectName("teamsDialog");
        teamsDialog->resize(400, 300);
        verticalLayoutWidget = new QWidget(teamsDialog);
        verticalLayoutWidget->setObjectName("verticalLayoutWidget");
        verticalLayoutWidget->setGeometry(QRect(10, 10, 381, 281));
        verticalLayout = new QVBoxLayout(verticalLayoutWidget);
        verticalLayout->setObjectName("verticalLayout");
        listTeams = new QListWidget(verticalLayoutWidget);
        listTeams->setObjectName("listTeams");
        verticalLayout->addWidget(listTeams);
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        edtName = new QLineEdit(verticalLayoutWidget);
        edtName->setObjectName("edtName");
        horizontalLayout->addWidget(edtName);
        cmbTournament = new QComboBox(verticalLayoutWidget);
        cmbTournament->setObjectName("cmbTournament");
        horizontalLayout->addWidget(cmbTournament);
        btnAdd = new QPushButton(verticalLayoutWidget);
        btnAdd->setObjectName("btnAdd");
        btnAdd->setText("Add");
        horizontalLayout->addWidget(btnAdd);
        verticalLayout->addLayout(horizontalLayout);

        horizontalLayout2 = new QHBoxLayout();
        horizontalLayout2->setObjectName("horizontalLayout2");
        btnEdit = new QPushButton(verticalLayoutWidget);
        btnEdit->setObjectName("btnEdit");
        btnEdit->setText("Editar");
        horizontalLayout2->addWidget(btnEdit);
        btnDelete = new QPushButton(verticalLayoutWidget);
        btnDelete->setObjectName("btnDelete");
        btnDelete->setText("Eliminar");
        horizontalLayout2->addWidget(btnDelete);
        btnRefresh = new QPushButton(verticalLayoutWidget);
        btnRefresh->setObjectName("btnRefresh");
        btnRefresh->setText("Actualizar");
        horizontalLayout2->addWidget(btnRefresh);
        verticalLayout->addLayout(horizontalLayout2);

        retranslateUi(teamsDialog);

        QMetaObject::connectSlotsByName(teamsDialog);
    }

    void retranslateUi(QDialog *teamsDialog)
    {
        teamsDialog->setWindowTitle(QCoreApplication::translate("teamsDialog", "Teams", nullptr));
    }

};

namespace Ui { class teamsDialog: public Ui_teamsDialog {}; }

QT_END_NAMESPACE
