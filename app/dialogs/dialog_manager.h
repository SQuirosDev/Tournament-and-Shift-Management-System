#pragma once

#include <QtWidgets/QDialog>

class DialogManager {
public:
    static void openModal(QDialog* dlg);
private:
    static QDialog* current_;
};
