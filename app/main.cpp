#include <iostream>
#include <QtWidgets/QApplication>
#include "app.h"
#include "connection.h"
#include "helpers.h"

using namespace std;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    app window;
    window.show();
    return a.exec();

    Connection connection;

    // Abrir la base de datos (se crea si no existe)
    DbResponse openResponse = connection.open("test_tournament.db");
    printResponse("open", openResponse);

    if (!connection.isOpen()) {
        qDebug() << "No se pudo abrir la base de datos.";
        return 1;
    }

    return 0;

}
