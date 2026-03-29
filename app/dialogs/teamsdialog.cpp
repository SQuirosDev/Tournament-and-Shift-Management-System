#include "dialogs/teamsdialog.h"
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QInputDialog>
#include <QtWidgets/QLineEdit>

using namespace std;
static QString toQString(const string &s) { return QString::fromStdString(s); }

teamsDialog::teamsDialog(Connection* conn, QWidget* parent)
    : QDialog(parent), conn_(conn)
{
    ui.setupUi(this);
    this->setFixedSize(600, 480);
    connect(ui.btnAdd, &QPushButton::clicked, this, &teamsDialog::onAddClicked);
    connect(ui.btnRefresh, &QPushButton::clicked, this, &teamsDialog::onRefresh);
    connect(ui.btnEdit, &QPushButton::clicked, this, &teamsDialog::onEditClicked);
    connect(ui.btnDelete, &QPushButton::clicked, this, &teamsDialog::onDeleteClicked);
    loadTournaments();
    loadTeams();
}

teamsDialog::~teamsDialog() {}

void teamsDialog::loadTournaments()
{
    ui.cmbTournament->clear();
    auto res = conn_->listTournaments();
    if (res.code < 0) return;
    for (auto &t : res.data) {
        ui.cmbTournament->addItem(QString::fromStdString(t.name), QVariant(t.id));
    }
}

void teamsDialog::loadTeams()
{
    ui.listTeams->clear();
    int tournamentId = ui.cmbTournament->currentData().toInt();
    if (tournamentId == 0) return;
    auto res = conn_->listTeamsByTournament(tournamentId);
    if (res.code < 0) return;
    for (auto &team : res.data) {
        ui.listTeams->addItem(QString::fromStdString(team.name) + " (id=" + QString::number(team.id) + ")");
    }
}

void teamsDialog::onAddClicked()
{
    string name = ui.edtName->text().toStdString();
    int tournamentId = ui.cmbTournament->currentData().toInt();
    if (name.empty()) {
        QMessageBox::warning(this, "Validation", "Name cannot be empty");
        return;
    }
    if (tournamentId == 0) {
        QMessageBox::warning(this, "Validation", "Select a tournament first");
        return;
    }
    auto resp = conn_->insertTeam(name, tournamentId);
    if (resp.code < 0) {
        QMessageBox::warning(this, "Error", QString::fromStdString(resp.message));
        return;
    }
    ui.edtName->clear();
    loadTeams();
}

void teamsDialog::onRefresh()
{
    loadTournaments();
    loadTeams();
}

void teamsDialog::onEditClicked()
{
    int row = ui.listTeams->currentRow();
    if (row < 0) { QMessageBox::information(this, "Editar", "Seleccione un equipo."); return; }
    QString item = ui.listTeams->currentItem()->text();
    int pos = item.indexOf("id="); if (pos == -1) return;
    QString num = item.mid(pos+3);
    QString digits; for (QChar c: num) { if (c.isDigit()) digits.append(c); else break; }
    int id = digits.toInt();
    auto res = conn_->obtainTeamById(id);
    if (res.code < 0) { QMessageBox::warning(this, "Error", "No se pudo obtener equipo."); return; }
    bool ok; QString newName = QInputDialog::getText(this, "Editar", "Nombre:", QLineEdit::Normal, QString::fromStdString(res.data[0].name), &ok);
    if (!ok || newName.trimmed().isEmpty()) return; conn_->updateTeam(id, newName.toStdString()); loadTeams();
}

void teamsDialog::onDeleteClicked()
{
    int row = ui.listTeams->currentRow();
    if (row < 0) { QMessageBox::information(this, "Eliminar", "Seleccione un equipo."); return; }
    QString item = ui.listTeams->currentItem()->text();
    int pos = item.indexOf("id="); if (pos == -1) return;
    QString num = item.mid(pos+3);
    QString digits; for (QChar c: num) { if (c.isDigit()) digits.append(c); else break; }
    int id = digits.toInt();
    auto confirm = QMessageBox::question(this, "Confirmar", "Eliminar equipo?");
    if (confirm != QMessageBox::Yes) return;
    conn_->deleteTeam(id);
    loadTeams();
}
