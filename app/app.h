#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_app.h"

class app : public QMainWindow
{
    Q_OBJECT

public:
    app(QWidget *parent = nullptr);
    ~app();

private:
    Ui::appClass ui;
};

