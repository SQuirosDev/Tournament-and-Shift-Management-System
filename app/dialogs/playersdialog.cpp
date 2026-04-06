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
    // center content with padding and allow list expansion
    ui.verticalLayoutWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui.listPlayers->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QVBoxLayout* outer = new QVBoxLayout(this);
    outer->setContentsMargins(40, 20, 40, 20);
    outer->addStretch();
    QWidget* centerWrapper = new QWidget(this);
    QHBoxLayout* hw = new QHBoxLayout(centerWrapper);
    hw->addStretch();
    centerWrapper->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    centerWrapper->setMaximumWidth(520);
    hw->addWidget(ui.verticalLayoutWidget);
    hw->addStretch();
    outer->addWidget(centerWrapper);
    outer->addStretch();
    connect(ui.btnAdd, &QPushButton::clicked, this, &playersDialog::onAddClicked);
    connect(ui.btnRefresh, &QPushButton::clicked, this, &playersDialog::onRefresh);
    connect(ui.btnEdit, &QPushButton::clicked, this, &playersDialog::onEditClicked);
    connect(ui.btnDelete, &QPushButton::clicked, this, &playersDialog::onDeleteClicked);
    connect(ui.cmbTeam, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &playersDialog::loadPlayers);
    loadTeams();
    loadPlayers();

    ui.listPlayers->setFocusPolicy(Qt::StrongFocus);
    ui.listPlayers->setSelectionMode(QAbstractItemView::SingleSelection);
    ui.listPlayers->setUniformItemSizes(true);
    ui.listPlayers->setMinimumHeight(220);
}

playersDialog::~playersDialog() {}

void playersDialog::loadTeams()
{
    ui.cmbTeam->clear();
    auto res = conn_->listAllTeams();
    if (res.code < 0) {
        // If listing teams fails, fall back to listing tournaments (older behavior)
        auto tres = conn_->listTournaments();
        if (tres.code < 0) return;
        for (auto &t : tres.data) ui.cmbTeam->addItem(QString::fromStdString(t.name), QVariant(t.id));
        return;
    }
    // populate teams combo with all teams (showing "Team (TournamentName)" may be better but we keep simple)
    for (auto &team : res.data) {
        ui.cmbTeam->addItem(QString::fromStdString(team.name) + " (t=" + QString::number(team.tournamentId) + ")", QVariant(team.id));
    }
    ui.listPlayers->setFrameShape(QFrame::Box);
    ui.listPlayers->setStyleSheet("background: transparent; padding: 6px;");
}

void playersDialog::loadPlayers()
{
    ui.listPlayers->clear();
    int teamId = ui.cmbTeam->currentData().toInt();
    if (teamId == 0) return;
    auto res = conn_->listPlayersByTeam(teamId);
    if (res.code < 0) return;
    for (auto &p : res.data) {
        QListWidgetItem* it = new QListWidgetItem(QString::fromStdString(p.name));
        it->setData(Qt::UserRole, p.id);
        it->setForeground(QBrush(QColor("#1d1d1f")));
        ui.listPlayers->addItem(it);
    }
    ui.listPlayers->setFrameShape(QFrame::Box);
    ui.listPlayers->setStyleSheet("background-color: #fbfbfd; color: #1d1d1f; padding: 6px; border-radius: 12px;");
    ui.listPlayers->update();
}

void playersDialog::onAddClicked()
{
    QString name = ui.edtName->text();
    if (name.trimmed().isEmpty()) {
        QMessageBox::warning(this, "Validación", "El nombre del jugador no puede estar vacío.");
        return;
    }
    int teamId = ui.cmbTeam->currentData().toInt();
    if (teamId == 0) {
        QMessageBox::warning(this, "Validación", "Seleccione un equipo (use el diálogo de Equipos para crear equipos primero).");
        return;
    }
    auto resp = conn_->insertPlayer(name.toStdString(), teamId);
    if (resp.code < 0) {
        QMessageBox::warning(this, "Error", QString::fromStdString(resp.message));
        return;
    }
    ui.edtName->clear();
    // Refresh players list after successful insertion
    loadPlayers();
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
    QListWidgetItem* it = ui.listPlayers->currentItem();
    if (!it) { QMessageBox::warning(this, "Error", "Seleccione un jugador."); return; }
    int id = it->data(Qt::UserRole).toInt();
    auto res = conn_->obtainPlayerById(id); if (res.code < 0) { QMessageBox::warning(this, "Error", "No se pudo obtener jugador."); return; }
    bool ok;
    QString newName = QInputDialog::getText(this, "Editar jugador", "Nombre del jugador:", QLineEdit::Normal, QString::fromStdString(res.data[0].name), &ok);
    if (!ok) return;
    if (newName.trimmed().isEmpty()) {
        QMessageBox::warning(this, "Validación", "El nombre no puede estar vacío.");
        return;
    }
    auto upr = conn_->updatePlayer(id, newName.toStdString());
    if (upr.code < 0) {
        QMessageBox::warning(this, "Error", QString::fromStdString(upr.message));
        return;
    }
    loadPlayers();
}

void playersDialog::onDeleteClicked()
{
    int row = ui.listPlayers->currentRow();
    if (row < 0) { QMessageBox::information(this, "Eliminar", "Seleccione un jugador."); return; }
    QListWidgetItem* it = ui.listPlayers->currentItem();
    if (!it) { QMessageBox::information(this, "Eliminar", "Seleccione un jugador."); return; }
    int id = it->data(Qt::UserRole).toInt();
    QMessageBox confirmBox(this);
    confirmBox.setIcon(QMessageBox::Question);
    confirmBox.setWindowTitle("Confirmar eliminación");
    confirmBox.setText("¿Eliminar al jugador seleccionado?");
    confirmBox.setInformativeText("Esta acción no se puede deshacer.");
    confirmBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    confirmBox.setDefaultButton(QMessageBox::No);
    int confirm = confirmBox.exec();
    if (confirm != QMessageBox::Yes) return;
    auto dr = conn_->deletePlayer(id);
    if (dr.code < 0) {
        QMessageBox::warning(this, "Error", QString::fromStdString(dr.message));
        return;
    }
    loadPlayers();
}
