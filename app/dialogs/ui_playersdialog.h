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

class Ui_playersDialog
{
public:
    QWidget *verticalLayoutWidget;
    QVBoxLayout *verticalLayout;
    QListWidget *listPlayers;
    QHBoxLayout *horizontalLayout;
    QLineEdit *edtName;
    QHBoxLayout *horizontalLayout2;
    QPushButton *btnEdit;
    QPushButton *btnDelete;
    QComboBox *cmbTeam;
    QPushButton *btnAdd;
    QPushButton *btnRefresh;

    void setupUi(QDialog *playersDialog)
    {
        if (playersDialog->objectName().isEmpty())
            playersDialog->setObjectName("playersDialog");
        playersDialog->resize(400, 300);
        verticalLayoutWidget = new QWidget(playersDialog);
        verticalLayoutWidget->setObjectName("verticalLayoutWidget");
        verticalLayoutWidget->setGeometry(QRect(10, 10, 381, 281));
        verticalLayout = new QVBoxLayout(verticalLayoutWidget);
        verticalLayout->setObjectName("verticalLayout");
        listPlayers = new QListWidget(verticalLayoutWidget);
        listPlayers->setObjectName("listPlayers");
        verticalLayout->addWidget(listPlayers);
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        edtName = new QLineEdit(verticalLayoutWidget);
        edtName->setObjectName("edtName");
        horizontalLayout->addWidget(edtName);
        cmbTeam = new QComboBox(verticalLayoutWidget);
        cmbTeam->setObjectName("cmbTeam");
        horizontalLayout->addWidget(cmbTeam);
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

        retranslateUi(playersDialog);

        QMetaObject::connectSlotsByName(playersDialog);
    }

    void retranslateUi(QDialog *playersDialog)
    {
        playersDialog->setWindowTitle(QCoreApplication::translate("playersDialog", "Players", nullptr));
    }

};

namespace Ui { class playersDialog: public Ui_playersDialog {}; }

QT_END_NAMESPACE
