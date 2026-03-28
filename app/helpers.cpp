#include <QDebug>
#include "helpers.h"

void printResponse(string functionName, DbResponse response) {
    qDebug() << "\n---" << QString::fromStdString(functionName) << "---";
    qDebug() << "ID :" << response.id;
    qDebug() << "Codigo :" << response.code;
    qDebug() << "Mensaje :" << QString::fromStdString(response.message);
}
