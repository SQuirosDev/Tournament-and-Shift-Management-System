#include "dialogs/playersdialog.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QLineEdit>

using namespace std;

static QString toQString(const string &s) { return QString::fromStdString(s); }
static string toStdString(const QString &s) { return s.toStdString(); }

playersDialog::playersDialog(Connection* conn, QWidget* parent)
    : QDialog(parent), conn_(conn)
{
    ui.setupUi(this);
    this->setFixedSize(600, 480);
    connect(ui.btnAdd, &QPushButton::clicked, this, &playersDialog::onAddClicked);
    connect(ui.btnRefresh, &QPushButton::clicked, this, &playersDialog::onRefresh);
    connect(ui.btnEdit, &QPushButton::clicked, this, &playersDialog::onEditClicked);
    connect(ui.btnDelete, &QPushButton::clicked, this, &playersDialog::onDeleteClicked);
    loadTeams();
    loadPlayers();
}

playersDialog::~playersDialog() {}

void playersDialog::loadTeams()
{
    ui.cmbTeam->clear();
    auto res = conn_->listTournaments();
    if (res.code < 0) return;
    // For simplicity, populate teams combo with tournaments as placeholder (teams list needs tournament context)
    for (auto &t : res.data) {
        ui.cmbTeam->addItem(QString::fromStdString(t.name), QVariant(t.id));
    }
}

void playersDialog::loadPlayers()
{
    ui.listPlayers->clear();
    // No team selected default: show nothing
}

void playersDialog::onAddClicked()
{
    QString name = ui.edtName->text();
    if (name.trimmed().isEmpty()) {
        QMessageBox::warning(this, "Validation", "Name cannot be empty");
        return;
    }
    int teamId = ui.cmbTeam->currentData().toInt();
    if (teamId == 0) {
        QMessageBox::warning(this, "Validation", "Select a team (use Teams dialog to create teams first)");
        return;
    }
    auto resp = conn_->insertPlayer(name.toStdString(), teamId);
    if (resp.code < 0) {
        QMessageBox::warning(this, "Error", QString::fromStdString(resp.message));
        return;
    }
    ui.edtName->clear();
}

void playersDialog::onRefresh()
{
    loadTeams();
    loadPlayers();
}

void playersDialog::onEditClicked()
{
    int row = ui.listPlayers->currentRow();
    if (row < 0) { QMessageBox::information(this, "Editar", "Seleccione un jugador."); return; }
    QString item = ui.listPlayers->currentItem()->text();
    int pos = item.indexOf("id="); if (pos == -1) return;
    QString num = item.mid(pos+3); QString digits; for (QChar c: num) { if (c.isDigit()) digits.append(c); else break; }
    int id = digits.toInt(); auto res = conn_->obtainPlayerById(id); if (res.code < 0) { QMessageBox::warning(this, "Error", "No se pudo obtener jugador."); return; }
    bool ok; QString newName = QInputDialog::getText(this, "Editar jugador", "Nombre:", QLineEdit::Normal, QString::fromStdString(res.data[0].name), &ok);
    if (!ok || newName.trimmed().isEmpty()) return; conn_->updatePlayer(id, newName.toStdString()); loadPlayers();
}

void playersDialog::onDeleteClicked()
{
    int row = ui.listPlayers->currentRow();
    if (row < 0) { QMessageBox::information(this, "Eliminar", "Seleccione un jugador."); return; }
    QString item = ui.listPlayers->currentItem()->text(); int pos = item.indexOf("id="); if (pos == -1) return; QString num = item.mid(pos+3); QString digits; for (QChar c: num) { if (c.isDigit()) digits.append(c); else break; } int id = digits.toInt();
    auto confirm = QMessageBox::question(this, "Confirmar", "Eliminar jugador?"); if (confirm != QMessageBox::Yes) return; conn_->deletePlayer(id); loadPlayers();
}
