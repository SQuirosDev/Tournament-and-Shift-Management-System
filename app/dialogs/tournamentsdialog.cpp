#include "dialogs/tournamentsdialog.h"
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QInputDialog>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QListWidgetItem>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QPushButton>

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
    // Center the dialog content with padding while allowing the list to expand
    ui.verticalLayoutWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui.listTournaments->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout* outer = new QVBoxLayout(this);
    outer->setContentsMargins(40, 20, 40, 20);
    outer->addStretch();
    QWidget* centerWrapper = new QWidget(this);
    centerWrapper->setStyleSheet("background: transparent;");
    ui.verticalLayoutWidget->setStyleSheet("background: transparent;");
    // Let layouts control geometry instead of the fixed geometry set by the ui file
    ui.verticalLayoutWidget->setGeometry(0,0,0,0);
    ui.verticalLayoutWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    ui.listTournaments->setMinimumHeight(240);
    QHBoxLayout* hw = new QHBoxLayout(centerWrapper);
    hw->addStretch();
    centerWrapper->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    centerWrapper->setMaximumWidth(520);
    hw->addWidget(ui.verticalLayoutWidget);
    hw->addStretch();
    outer->addWidget(centerWrapper);
    outer->addStretch();


    // Wire up signals
    connect(ui.btnAdd, &QPushButton::clicked, this, &tournamentsDialog::onAddClicked);
    connect(ui.btnRefresh, &QPushButton::clicked, this, &tournamentsDialog::onRefresh);
    connect(ui.btnEdit, &QPushButton::clicked, this, &tournamentsDialog::onEditClicked);
    connect(ui.btnDelete, &QPushButton::clicked, this, &tournamentsDialog::onDeleteClicked);

    // Runtime phase controls (combo + action button) inserted into the existing actions layout
    cmbPhase_ = new QComboBox(ui.verticalLayoutWidget);
    cmbPhase_->addItem("Grupos");
    cmbPhase_->addItem("Eliminacion");
    btnChangePhase_ = new QPushButton("Cambiar Fase", ui.verticalLayoutWidget);
    btnChangePhase_->setMinimumWidth(120);
    // disabled until a tournament is selected
    btnChangePhase_->setEnabled(false);
    // Insert widgets into the second horizontal layout that already contains edit/delete/refresh
    if (ui.horizontalLayout2) {
        ui.horizontalLayout2->addWidget(cmbPhase_);
        ui.horizontalLayout2->addWidget(btnChangePhase_);
    }
    connect(btnChangePhase_, &QPushButton::clicked, this, &tournamentsDialog::onChangePhaseClicked);

    // Update phase combobox when the current item changes
    connect(ui.listTournaments, &QListWidget::currentItemChanged, this, [this](QListWidgetItem* curr, QListWidgetItem* /*prev*/){
        if (!curr) {
            if (btnChangePhase_) btnChangePhase_->setEnabled(false);
            return;
        }
        int id = curr->data(Qt::UserRole).toInt();
        if (id <= 0) return;
        auto res = conn_->obtainTournamentById(id);
        if (res.code < 0 || res.data.empty()) return;
        QString phase = QString::fromStdString(res.data[0].phase);
        int idx = cmbPhase_->findText(phase);
        if (idx >= 0) cmbPhase_->setCurrentIndex(idx);
        if (btnChangePhase_) btnChangePhase_->setEnabled(true);
    });

    // set texts in Spanish and placeholders
    ui.btnAdd->setText("Crear");
    ui.btnRefresh->setText("Actualizar");
    ui.edtName->setPlaceholderText("Nombre del torneo...");

    loadTournaments();

    // Ensure list has expected focus and selection behavior
    ui.listTournaments->setFocusPolicy(Qt::StrongFocus);
    ui.listTournaments->setSelectionMode(QAbstractItemView::SingleSelection);
    ui.listTournaments->setUniformItemSizes(true);
    ui.listTournaments->setMinimumHeight(240);
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
        QListWidgetItem* it = new QListWidgetItem(toQString(t.name) + " [" + toQString(t.phase) + "]");
        it->setData(Qt::UserRole, t.id);
        it->setForeground(QBrush(QColor("#1d1d1f")));
        ui.listTournaments->addItem(it);
    }
    ui.listTournaments->setFrameShape(QFrame::Box);
    ui.listTournaments->setStyleSheet("background-color: #fbfbfd; color: #1d1d1f; padding: 6px; border-radius: 12px;");
    ui.listTournaments->update();
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
    // Select the newly created item and open teams dialog flow
    loadTournaments();
    // Find the item with the inserted id and select it
    for (int i = 0; i < ui.listTournaments->count(); ++i) {
        QListWidgetItem* it = ui.listTournaments->item(i);
        if (it && it->data(Qt::UserRole).toInt() == resp.id) {
            ui.listTournaments->setCurrentItem(it);
            break;
        }
    }
}

void tournamentsDialog::onRefresh()
{
    loadTournaments();
}

void tournamentsDialog::onChangePhaseClicked()
{
    QListWidgetItem* it = ui.listTournaments->currentItem();
    if (!it) {
        QMessageBox::information(this, "Cambiar fase", "Seleccione un torneo para cambiar la fase.");
        return;
    }
    int id = it->data(Qt::UserRole).toInt();
    if (id <= 0) return;
    QString phase = cmbPhase_ ? cmbPhase_->currentText() : QString();
    if (phase.isEmpty()) return;
    auto resp = conn_->updateTournamentPhase(id, phase.toStdString());
    if (resp.code < 0) {
        QMessageBox::warning(this, "Error", QString::fromStdString(resp.message));
        return;
    }
    // Refresh list and reselect updated item
    loadTournaments();
    for (int i = 0; i < ui.listTournaments->count(); ++i) {
        QListWidgetItem* nit = ui.listTournaments->item(i);
        if (nit && nit->data(Qt::UserRole).toInt() == id) {
            ui.listTournaments->setCurrentItem(nit);
            break;
        }
    }
}

void tournamentsDialog::onEditClicked()
{
    int row = ui.listTournaments->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Editar", "Seleccione un torneo para editar.");
        return;
    }
    QListWidgetItem* it = ui.listTournaments->currentItem();
    if (!it) return;
    int id = it->data(Qt::UserRole).toInt();
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
    ui.listTournaments->repaint();
    ui.listTournaments->update();
}

void tournamentsDialog::onDeleteClicked()
{
    int row = ui.listTournaments->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Eliminar", "Seleccione un torneo para eliminar.");
        return;
    }
    QListWidgetItem* it = ui.listTournaments->currentItem();
    if (!it) return;
    int id = it->data(Qt::UserRole).toInt();
    if (id <= 0) return;
    QMessageBox confirmBox(this);
    confirmBox.setIcon(QMessageBox::Question);
    confirmBox.setWindowTitle("Confirmar eliminación");
    confirmBox.setText("¿Eliminar el torneo seleccionado?");
    confirmBox.setInformativeText("Esta acción no se puede deshacer.");
    confirmBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    confirmBox.setDefaultButton(QMessageBox::No);
    int answer = confirmBox.exec();
    if (answer != QMessageBox::Yes) return;
    auto resp = conn_->deleteTournament(id);
    if (resp.code < 0) {
        QMessageBox::warning(this, "Error", QString::fromStdString(resp.message));
        return;
    }
    loadTournaments();
}
