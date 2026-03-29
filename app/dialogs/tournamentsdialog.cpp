#include "dialogs/tournamentsdialog.h"
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QInputDialog>

using namespace std;

static QString toQString(const string &s) { return QString::fromStdString(s); }
static string toStdString(const QString &s) { return s.toStdString(); }


tournamentsDialog::tournamentsDialog(Connection* conn, QWidget* parent)
    : QDialog(parent), conn_(conn)
{
    ensureUi();
}


tournamentsDialog::~tournamentsDialog() {}


void tournamentsDialog::ensureUi()
{
    ui.setupUi(this);
    // Normalize size and center handled by DialogManager
    this->setFixedSize(600, 480);

    // Wire up signals
    connect(ui.btnAdd, &QPushButton::clicked, this, &tournamentsDialog::onAddClicked);
    connect(ui.btnRefresh, &QPushButton::clicked, this, &tournamentsDialog::onRefresh);

    // set texts in Spanish and placeholders
    ui.btnAdd->setText("Crear");
    ui.btnRefresh->setText("Actualizar");
    ui.edtName->setPlaceholderText("Nombre del torneo...");

    loadTournaments();
}

void tournamentsDialog::loadTournaments()
{
    ui.listTournaments->clear();
    auto res = conn_->listTournaments();
    if (res.code < 0) {
        QMessageBox::warning(this, "Error", QString::fromStdString(res.message));
        return;
    }
    for (auto &t : res.data) {
        ui.listTournaments->addItem(toQString(t.name) + " [" + toQString(t.phase) + "] (id=" + QString::number(t.id) + ")");
    }
}

void tournamentsDialog::onAddClicked()
{
    QString name = ui.edtName->text().trimmed();
    if (name.isEmpty()) {
        QMessageBox::warning(this, "Validación", "El nombre no puede estar vacío.");
        return;
    }
    auto resp = conn_->insertTournament(name.toStdString());
    if (resp.code < 0) {
        QMessageBox::warning(this, "Error", QString::fromStdString(resp.message));
        return;
    }
    ui.edtName->clear();
    loadTournaments();
}

void tournamentsDialog::onRefresh()
{
    loadTournaments();
}

void tournamentsDialog::onEditClicked()
{
    int row = ui.listTournaments->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Editar", "Seleccione un torneo para editar.");
        return;
    }
    // Parse id from item text (simple parse since format is name [phase] (id=X))
    QString itemText = ui.listTournaments->currentItem()->text();
    // Extract id using simple parsing
    int id = -1;
    int pos = itemText.indexOf("id=");
    if (pos != -1) {
        QString num = itemText.mid(pos + 3);
        // remove non-digits
        QString digits;
        for (QChar c : num) { if (c.isDigit()) digits.append(c); else break; }
        id = digits.toInt();
    }
    if (id <= 0) return;
    auto res = conn_->obtainTournamentById(id);
    if (res.code < 0) {
        QMessageBox::warning(this, "Error", "No se pudo obtener el torneo.");
        return;
    }
    bool ok;
    QString newName = QInputDialog::getText(this, "Editar torneo", "Nombre:", QLineEdit::Normal, QString::fromStdString(res.data[0].name), &ok);
    if (!ok || newName.trimmed().isEmpty()) return;
    conn_->updateTournamentName(id, newName.toStdString());
    loadTournaments();
}

void tournamentsDialog::onDeleteClicked()
{
    int row = ui.listTournaments->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Eliminar", "Seleccione un torneo para eliminar.");
        return;
    }
    QString itemText = ui.listTournaments->currentItem()->text();
    int id = -1;
    int pos = itemText.indexOf("id=");
    if (pos != -1) {
        QString num = itemText.mid(pos + 3);
        QString digits;
        for (QChar c : num) { if (c.isDigit()) digits.append(c); else break; }
        id = digits.toInt();
    }
    if (id <= 0) return;
    auto confirm = QMessageBox::question(this, "Confirmar", "¿Eliminar este torneo? Esta acción no se puede deshacer.");
    if (confirm != QMessageBox::Yes) return;
    // If Connection supports deleteTournament, use it; otherwise mark as TODO
    // conn_->deleteTournament(id);
    // fallback: show not implemented message
    QMessageBox::information(this, "Info", "Eliminación no implementada en la capa de conexión.");
    loadTournaments();
}
