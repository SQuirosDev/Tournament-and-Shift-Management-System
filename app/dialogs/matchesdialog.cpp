#include "dialogs/matchesdialog.h"
#include <QtWidgets/QMessageBox>

using namespace std;

matchesDialog::matchesDialog(Connection* conn, QWidget* parent)
    : QDialog(parent), conn_(conn)
{
    ui.setupUi(this);
    // center content with padding and allow list expansion
    ui.verticalLayoutWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui.listMatches->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QVBoxLayout* outer = new QVBoxLayout(this);
    outer->setContentsMargins(40, 20, 40, 20);
    outer->addStretch();
    QWidget* centerWrapper = new QWidget(this);
    QHBoxLayout* hw = new QHBoxLayout(centerWrapper);
    hw->addStretch();
    centerWrapper->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    centerWrapper->setMaximumWidth(620);
    hw->addWidget(ui.verticalLayoutWidget);
    hw->addStretch();
    outer->addWidget(centerWrapper);
    outer->addStretch();
    connect(ui.btnAdd, &QPushButton::clicked, this, &matchesDialog::onAddClicked);
    connect(ui.btnRefresh, &QPushButton::clicked, this, &matchesDialog::onRefresh);
    connect(ui.cmbTournament, &QComboBox::currentIndexChanged, this, [this](int){
        int id = ui.cmbTournament->currentData().toInt();
        loadTeamsForTournament(id);
        loadMatches(id);
    });
    loadTournaments();
}

matchesDialog::~matchesDialog() {}

void matchesDialog::loadTournaments()
{
    ui.cmbTournament->clear();
    auto res = conn_->listTournaments();
    if (res.code < 0) return;
    for (auto &t : res.data) {
        ui.cmbTournament->addItem(QString::fromStdString(t.name), QVariant(t.id));
    }
}

void matchesDialog::loadTeamsForTournament(int tournamentId)
{
    ui.cmbTeamA->clear();
    ui.cmbTeamB->clear();
    if (tournamentId == 0) return;
    auto res = conn_->listTeamsByTournament(tournamentId);
    if (res.code < 0) return;
    for (auto &team : res.data) {
        ui.cmbTeamA->addItem(QString::fromStdString(team.name), QVariant(team.id));
        ui.cmbTeamB->addItem(QString::fromStdString(team.name), QVariant(team.id));
    }
}

void matchesDialog::loadMatches(int tournamentId)
{
    ui.listMatches->clear();
    if (tournamentId == 0) return;
    auto res = conn_->listMatchesByTournament(tournamentId);
    if (res.code < 0) return;
    for (auto &m : res.data) {
        QListWidgetItem* it = new QListWidgetItem("(" + QString::number(m.id) + ") " + QString::fromStdString(m.result) + " - " + QString::fromStdString(m.phase));
        it->setData(Qt::UserRole, m.id);
        it->setForeground(QBrush(QColor("#1d1d1f")));
        ui.listMatches->addItem(it);
    }
    ui.listMatches->setFrameShape(QFrame::Box);
    ui.listMatches->setStyleSheet("background-color: #fbfbfd; color: #1d1d1f; padding: 6px; border-radius: 12px;");
    ui.listMatches->update();
}

void matchesDialog::onAddClicked()
{
    int tournamentId = ui.cmbTournament->currentData().toInt();
    int teamA = ui.cmbTeamA->currentData().toInt();
    int teamB = ui.cmbTeamB->currentData().toInt();
    if (tournamentId == 0 || teamA == 0 || teamB == 0) {
        QMessageBox::warning(this, "Validación", "Seleccione un torneo y ambos equipos.");
        return;
    }
    if (teamA == teamB) {
        QMessageBox::warning(this, "Validación", "Los equipos deben ser diferentes.");
        return;
    }
    auto resp = conn_->insertMatch(tournamentId, teamA, teamB, "Grupos", 1, 0);
    if (resp.code < 0) {
        QMessageBox::warning(this, "Error", QString::fromStdString(resp.message));
        return;
    }
    loadMatches(tournamentId);
}

void matchesDialog::onRefresh()
{
    loadTournaments();
    int id = ui.cmbTournament->currentData().toInt();
    loadTeamsForTournament(id);
    loadMatches(id);
}
