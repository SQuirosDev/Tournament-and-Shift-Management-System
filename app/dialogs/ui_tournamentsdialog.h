#pragma once

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_tournamentsDialog
{
public:
    QWidget *verticalLayoutWidget;
    QVBoxLayout *verticalLayout;
    QListWidget *listTournaments;
    QHBoxLayout *horizontalLayout;
    QLineEdit *edtName;
    QHBoxLayout *horizontalLayout2;
    QPushButton *btnEdit;
    QPushButton *btnDelete;
    QPushButton *btnAdd;
    QPushButton *btnRefresh;

    void setupUi(QDialog *tournamentsDialog)
    {
        if (tournamentsDialog->objectName().isEmpty())
            tournamentsDialog->setObjectName("tournamentsDialog");
        tournamentsDialog->resize(400, 300);
        verticalLayoutWidget = new QWidget(tournamentsDialog);
        verticalLayoutWidget->setObjectName("verticalLayoutWidget");
        verticalLayoutWidget->setGeometry(QRect(10, 10, 381, 281));
        verticalLayout = new QVBoxLayout(verticalLayoutWidget);
        verticalLayout->setObjectName("verticalLayout");
        listTournaments = new QListWidget(verticalLayoutWidget);
        listTournaments->setObjectName("listTournaments");
        verticalLayout->addWidget(listTournaments);
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        edtName = new QLineEdit(verticalLayoutWidget);
        edtName->setObjectName("edtName");
        horizontalLayout->addWidget(edtName);
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

        retranslateUi(tournamentsDialog);

        QMetaObject::connectSlotsByName(tournamentsDialog);
    }

    void retranslateUi(QDialog *tournamentsDialog)
    {
        tournamentsDialog->setWindowTitle(QCoreApplication::translate("tournamentsDialog", "Tournaments", nullptr));
    }

};

namespace Ui { class tournamentsDialog: public Ui_tournamentsDialog {}; }

QT_END_NAMESPACE
