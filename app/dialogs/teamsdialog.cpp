#include "dialogs/teamsdialog.h"
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QInputDialog>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QSizePolicy>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QListWidgetItem>

using namespace std;
static QString toQString(const string &s) { return QString::fromStdString(s); }

teamsDialog::teamsDialog(Connection* conn, QWidget* parent)
    : QDialog(parent), conn_(conn)
{
    ui.setupUi(this);
    this->setFixedSize(600, 480);
    // center content with padding and allow list expansion
    ui.verticalLayoutWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui.listTeams->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
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
    connect(ui.btnAdd, &QPushButton::clicked, this, &teamsDialog::onAddClicked);
    // Refresh button not needed: lists refresh automatically on changes
    if (ui.btnRefresh) ui.btnRefresh->hide();
    // Ensure teams reload when tournament selection changes
    connect(ui.cmbTournament, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &teamsDialog::loadTeams);
    connect(ui.btnEdit, &QPushButton::clicked, this, &teamsDialog::onEditClicked);
    connect(ui.btnDelete, &QPushButton::clicked, this, &teamsDialog::onDeleteClicked);
    loadTournaments();
    loadTeams();

    ui.listTeams->setFocusPolicy(Qt::StrongFocus);
    ui.listTeams->setSelectionMode(QAbstractItemView::SingleSelection);
    ui.listTeams->setUniformItemSizes(true);
    ui.listTeams->setMinimumHeight(220);
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
    // ensure we trigger team load on initial population
    if (ui.cmbTournament->count() > 0) ui.cmbTournament->setCurrentIndex(0);
}

void teamsDialog::loadTeams()
{
    ui.listTeams->clear();
    int tournamentId = ui.cmbTournament->currentData().toInt();
    if (tournamentId == 0) return;
    auto res = conn_->listTeamsByTournament(tournamentId);
    if (res.code < 0) return;
    for (auto &team : res.data) {
        QListWidgetItem* it = new QListWidgetItem(QString::fromStdString(team.name));
        it->setData(Qt::UserRole, team.id);
        it->setForeground(QBrush(QColor("#1d1d1f")));
        ui.listTeams->addItem(it);
    }
    ui.listTeams->setFrameShape(QFrame::Box);
    ui.listTeams->setStyleSheet("background-color: #fbfbfd; color: #1d1d1f; padding: 6px; border-radius: 12px;");
    ui.listTeams->update();
}

void teamsDialog::onAddClicked()
{
    string name = ui.edtName->text().toStdString();
    int tournamentId = ui.cmbTournament->currentData().toInt();
    if (name.empty()) {
        QMessageBox::warning(this, "Validación", "El nombre del equipo no puede estar vacío.");
        return;
    }
    if (tournamentId == 0) {
        QMessageBox::warning(this, "Validación", "Seleccione un torneo primero.");
        return;
    }
    auto resp = conn_->insertTeam(name, tournamentId);
    if (resp.code < 0) {
        QMessageBox::warning(this, "Error", QString::fromStdString(resp.message));
        return;
    }
    ui.edtName->clear();
    // Refresh teams list after successful insertion
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
    QListWidgetItem* it = ui.listTeams->currentItem();
    if (!it) { QMessageBox::warning(this, "Error", "Seleccione un equipo."); return; }
    int id = it->data(Qt::UserRole).toInt();
    auto res = conn_->obtainTeamById(id);
    if (res.code < 0) { QMessageBox::warning(this, "Error", "No se pudo obtener el equipo."); return; }
    bool ok; QString newName = QInputDialog::getText(this, "Editar equipo", "Nombre del equipo:", QLineEdit::Normal, QString::fromStdString(res.data[0].name), &ok);
    if (!ok) return;
    if (newName.trimmed().isEmpty()) { QMessageBox::warning(this, "Validación", "El nombre no puede estar vacío."); return; }
    auto upr = conn_->updateTeam(id, newName.toStdString());
    if (upr.code < 0) { QMessageBox::warning(this, "Error", QString::fromStdString(upr.message)); return; }
    loadTeams();
}

void teamsDialog::onDeleteClicked()
{
    int row = ui.listTeams->currentRow();
    if (row < 0) { QMessageBox::information(this, "Eliminar", "Seleccione un equipo."); return; }
    QListWidgetItem* it = ui.listTeams->currentItem();
    if (!it) return;
    int id = it->data(Qt::UserRole).toInt();
    QMessageBox confirmBox(this);
    confirmBox.setIcon(QMessageBox::Question);
    confirmBox.setWindowTitle("Confirmar eliminación");
    confirmBox.setText("¿Eliminar el equipo seleccionado?");
    confirmBox.setInformativeText("Esta acción no se puede deshacer.");
    confirmBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    confirmBox.setDefaultButton(QMessageBox::No);
    int confirm = confirmBox.exec();
    if (confirm != QMessageBox::Yes) return;
    auto dr = conn_->deleteTeam(id);
    if (dr.code < 0) { QMessageBox::warning(this, "Error", QString::fromStdString(dr.message)); return; }
    loadTeams();
}
