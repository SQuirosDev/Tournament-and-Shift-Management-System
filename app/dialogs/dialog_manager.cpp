#include "dialog_manager.h"
#include <QtWidgets/QApplication>
#include <QtGui/QScreen>

QDialog* DialogManager::current_ = nullptr;

void DialogManager::openModal(QDialog* dlg)
{
    if (current_) {
        current_->close();
        current_ = nullptr;
    }
    current_ = dlg;
    dlg->setModal(true);
    // center on screen
    QScreen* screen = QApplication::primaryScreen();
    QRect geo = screen ? screen->availableGeometry() : QRect(0,0,800,600);
    dlg->resize(600, 480);
    int x = geo.x() + (geo.width() - dlg->width())/2;
    int y = geo.y() + (geo.height() - dlg->height())/2;
    dlg->move(x,y);
    dlg->exec();
    current_ = nullptr;
}
