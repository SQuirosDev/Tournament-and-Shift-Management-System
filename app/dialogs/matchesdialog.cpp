#include "dialogs/matchesdialog.h"
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QInputDialog>
#include <QMenu>
#include <QAction>
#include <QtWidgets/QDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QDialogButtonBox>

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
    // Hide refresh button; lists refresh automatically after changes
    if (ui.btnRefresh) ui.btnRefresh->hide();
    connect(ui.cmbTournament, &QComboBox::currentIndexChanged, this, [this](int){
        int id = ui.cmbTournament->currentData().toInt();
        loadTeamsForTournament(id);
        loadMatches(id);
    });
    connect(ui.btnAdd, &QPushButton::clicked, this, &matchesDialog::onAddClicked);
    // Add edit/delete handlers using context menu style or buttons if present
    // We'll connect list double-click to edit and context menu for delete
    connect(ui.listMatches, &QListWidget::itemDoubleClicked, this, &matchesDialog::onEditClicked);
    ui.listMatches->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui.listMatches, &QWidget::customContextMenuRequested, this, [this](const QPoint &pt){
        QListWidgetItem* it = ui.listMatches->itemAt(pt);
        if (!it) return;
        QMenu menu(this);
        QAction *editAct = menu.addAction("Editar resultado");
        QAction *delAct = menu.addAction("Eliminar partido");
        QAction *sel = menu.exec(ui.listMatches->mapToGlobal(pt));
        if (sel == editAct) onEditClicked();
        else if (sel == delAct) onDeleteClicked();
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
    QMessageBox::information(this, "Creado", "Partido creado correctamente.");
}

void matchesDialog::onEditClicked()
{
    QListWidgetItem* it = ui.listMatches->currentItem();
    if (!it) {
        QMessageBox::information(this, "Editar", "Seleccione un partido para editar.");
        return;
    }
    int id = it->data(Qt::UserRole).toInt();
    if (id <= 0) return;
    auto res = conn_->obtainMatchById(id);
    if (res.code < 0 || res.data.empty()) {
        QMessageBox::warning(this, "Error", "No se pudo obtener el partido.");
        return;
    }
    auto m = res.data[0];
    // Open a richer edit dialog to modify phase, round and result
    QDialog dlg(this);
    dlg.setWindowTitle("Editar Partido");
    QFormLayout *form = new QFormLayout(&dlg);

    QComboBox *cmbPhase = new QComboBox(&dlg);
    cmbPhase->addItem("Grupos"); cmbPhase->addItem("Eliminacion");
    int phaseIdx = cmbPhase->findText(QString::fromStdString(m.phase));
    if (phaseIdx >= 0) cmbPhase->setCurrentIndex(phaseIdx);

    QSpinBox *spRound = new QSpinBox(&dlg);
    spRound->setMinimum(1);
    spRound->setMaximum(999);
    spRound->setValue(m.round);

    QComboBox *cmbResult = new QComboBox(&dlg);
    cmbResult->addItem("Pendiente"); cmbResult->addItem("Gana A"); cmbResult->addItem("Gana B"); cmbResult->addItem("Empate");
    int resIdx = cmbResult->findText(QString::fromStdString(m.result));
    if (resIdx >= 0) cmbResult->setCurrentIndex(resIdx);

    form->addRow(new QLabel("Fase:"), cmbPhase);
    form->addRow(new QLabel("Ronda:"), spRound);
    form->addRow(new QLabel("Resultado:"), cmbResult);

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    form->addRow(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    if (dlg.exec() != QDialog::Accepted) return;

    string newPhase = cmbPhase->currentText().toStdString();
    int newRound = spRound->value();
    string newResult = cmbResult->currentText().toStdString();
    int winnerId = 0;
    if (newResult == "Gana A") winnerId = m.teamAId;
    else if (newResult == "Gana B") winnerId = m.teamBId;
    string status = (newResult == "Pendiente") ? "Pendiente" : "Finalizado";

    // call updateMatch: (int id, string phase, int round, string status, int winnerId, string result)
    auto upd = conn_->updateMatch(id, newPhase, newRound, status, winnerId, newResult);
    if (upd.code < 0) {
        QMessageBox::warning(this, "Error", QString::fromStdString(upd.message));
        return;
    }

    // If finalized, update team stats accordingly
    if (status == "Finalizado") {
        // Update team stats: win/draw/loss and points (3/1/0)
        auto teamAres = conn_->obtainTeamById(m.teamAId);
        auto teamBres = conn_->obtainTeamById(m.teamBId);
        if (teamAres.code >= 0 && !teamAres.data.empty() && teamBres.code >= 0 && !teamBres.data.empty()) {
            Team ta = teamAres.data[0];
            Team tb = teamBres.data[0];
            if (newResult == "Gana A") {
                ta.wins += 1; ta.points += 3;
                tb.losses += 1;
            } else if (newResult == "Gana B") {
                tb.wins += 1; tb.points += 3;
                ta.losses += 1;
            } else if (newResult == "Empate") {
                ta.draws += 1; ta.points += 1;
                tb.draws += 1; tb.points += 1;
            }
            conn_->updateTeamStats(ta.id, ta.points, ta.wins, ta.draws, ta.losses);
            conn_->updateTeamStats(tb.id, tb.points, tb.wins, tb.draws, tb.losses);
        }
    }
    // refresh
    int tournamentId = ui.cmbTournament->currentData().toInt();
    loadMatches(tournamentId);
}

void matchesDialog::onDeleteClicked()
{
    QListWidgetItem* it = ui.listMatches->currentItem();
    if (!it) {
        QMessageBox::information(this, "Eliminar", "Seleccione un partido para eliminar.");
        return;
    }
    int id = it->data(Qt::UserRole).toInt();
    if (id <= 0) return;
    QMessageBox confirm(this);
    confirm.setIcon(QMessageBox::Question);
    confirm.setWindowTitle("Confirmar eliminación");
    confirm.setText("¿Eliminar el partido seleccionado?");
    confirm.setInformativeText("Esta acción no se puede deshacer.");
    confirm.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    confirm.setDefaultButton(QMessageBox::No);
    int ans = confirm.exec();
    if (ans != QMessageBox::Yes) return;
    auto resp = conn_->deleteMatch(id);
    if (resp.code < 0) {
        QMessageBox::warning(this, "Error", QString::fromStdString(resp.message));
        return;
    }
    int tournamentId = ui.cmbTournament->currentData().toInt();
    loadMatches(tournamentId);
    QMessageBox::information(this, "Eliminado", "Partido eliminado correctamente.");
}

void matchesDialog::onRefresh()
{
    loadTournaments();
    int id = ui.cmbTournament->currentData().toInt();
    loadTeamsForTournament(id);
    loadMatches(id);
}
