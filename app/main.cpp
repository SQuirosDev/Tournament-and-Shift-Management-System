#include <iostream>
#include <QtWidgets/QApplication>
#include "app.h"
#include "connection.h"
#include "helpers.h"

using namespace std;

int main(int argc, char *argv[])
{
    // Ejecutar la aplicación GUI por defecto
    QApplication a(argc, argv);
    app window;
    window.show();
    return a.exec();
}
