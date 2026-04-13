#include "dialogs/teamsdialog.h"
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QWidget>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QInputDialog>

static const char* SS_HEADER = "background:#ffffff;border-bottom:1px solid #e0e0e8;";
static const char* SS_BODY = "background:#f5f5f7;";
static const char* SS_BAR = "background:#ffffff;border:1px solid #e0e0e8;border-radius:12px;";
static const char* SS_ICO = "background:rgba(0,113,227,0.10);border-radius:10px;font-size:15px;font-weight:700;color:#0071e3;";
static const char* SS_PRIMARY = "QPushButton{background:#0071e3;color:#fff;border:none;border-radius:9px;padding:8px 18px;font-size:13px;font-weight:600;}QPushButton:hover{background:#0077ed;}QPushButton:disabled{background:#b3d4f5;}";
static const char* SS_SEC = "QPushButton{background:#ffffff;color:#1d1d1f;border:1px solid #d2d2d7;border-radius:9px;padding:8px 18px;font-size:13px;font-weight:600;}QPushButton:hover{background:#f0f0f5;}QPushButton:disabled{color:#b0b0b8;}";
static const char* SS_DANGER = "QPushButton{background:#ff3b30;color:#fff;border:none;border-radius:9px;padding:8px 18px;font-size:13px;font-weight:600;}QPushButton:hover{background:#ff453a;}QPushButton:disabled{background:#ffb3af;}";

teamsDialog::teamsDialog(LogTeam* logTeam, LogTournament* logTournament, QWidget* parent)
    : QDialog(parent), logTeam_(logTeam), logTournament_(logTournament)
{
    setWindowTitle("Gestionar Equipos");
    setFixedSize(580, 500);

    QVBoxLayout* root = new QVBoxLayout(this);
    root->setSpacing(0); root->setContentsMargins(0, 0, 0, 0);

    // Header
    QWidget* hdr = new QWidget(); hdr->setStyleSheet(SS_HEADER);
    QHBoxLayout* hL = new QHBoxLayout(hdr);
    hL->setContentsMargins(22, 14, 22, 14); hL->setSpacing(12);
    QLabel* ico = new QLabel("E"); ico->setFixedSize(38, 38);
    ico->setAlignment(Qt::AlignCenter); ico->setStyleSheet(SS_ICO);
    QVBoxLayout* tc = new QVBoxLayout(); tc->setSpacing(1);
    QLabel* lt = new QLabel("Equipos");
    lt->setStyleSheet("font-size:14px;font-weight:700;color:#1d1d1f;");
    lblSubtitle_ = new QLabel();
    lblSubtitle_->setStyleSheet("font-size:11px;color:#86868b;");
    tc->addWidget(lt); tc->addWidget(lblSubtitle_);
    hL->addWidget(ico); hL->addLayout(tc); hL->addStretch();
    root->addWidget(hdr);

    // Body
    QWidget* body = new QWidget(); body->setStyleSheet(SS_BODY);
    QVBoxLayout* bL = new QVBoxLayout(body);
    bL->setContentsMargins(18, 14, 18, 14); bL->setSpacing(10);

    // Barra agregar
    QWidget* addBar = new QWidget(); addBar->setStyleSheet(SS_BAR);
    QHBoxLayout* aL = new QHBoxLayout(addBar);
    aL->setContentsMargins(12, 8, 12, 8); aL->setSpacing(8);
    edtName_ = new QLineEdit(); edtName_->setPlaceholderText("Nombre del equipo...");
    cmbTournament_ = new QComboBox(); cmbTournament_->setMinimumWidth(160);
    QPushButton* btnAdd = new QPushButton("Agregar");
    btnAdd->setStyleSheet(SS_PRIMARY); btnAdd->setFixedHeight(36);
    aL->addWidget(edtName_, 1); aL->addWidget(cmbTournament_); aL->addWidget(btnAdd);
    bL->addWidget(addBar);

    // Lista
    listTeams_ = new QListWidget();
    listTeams_->setSelectionMode(QAbstractItemView::SingleSelection);
    bL->addWidget(listTeams_, 1);

    // Toolbar
    QHBoxLayout* tL = new QHBoxLayout(); tL->setSpacing(8);
    btnEdit_ = new QPushButton("Editar nombre"); btnEdit_->setStyleSheet(SS_SEC);
    btnDelete_ = new QPushButton("Eliminar");      btnDelete_->setStyleSheet(SS_DANGER);
    QPushButton* btnR = new QPushButton("Actualizar"); btnR->setStyleSheet(SS_SEC);
    btnEdit_->setEnabled(false); btnDelete_->setEnabled(false);
    tL->addWidget(btnEdit_); tL->addWidget(btnDelete_); tL->addStretch(); tL->addWidget(btnR);
    bL->addLayout(tL);
    root->addWidget(body, 1);

    connect(btnAdd, &QPushButton::clicked, this, &teamsDialog::onAddClicked);
    connect(btnEdit_, &QPushButton::clicked, this, &teamsDialog::onEditClicked);
    connect(btnDelete_, &QPushButton::clicked, this, &teamsDialog::onDeleteClicked);
    connect(btnR, &QPushButton::clicked, this, &teamsDialog::onRefresh);
    connect(edtName_, &QLineEdit::returnPressed, this, &teamsDialog::onAddClicked);
    connect(listTeams_, &QListWidget::itemSelectionChanged, this, [this]() {
        bool s = !listTeams_->selectedItems().isEmpty();
        btnEdit_->setEnabled(s); btnDelete_->setEnabled(s);
        });

    loadTournaments(); loadTeams();
}
teamsDialog::~teamsDialog() {}

void teamsDialog::loadTournaments()
{
    cmbTournament_->clear();
    cmbTournament_->addItem("— Torneo —", -1);
    for (const auto& t : logTournament_->list().data)
        cmbTournament_->addItem(QString::fromStdString(t.name), t.id);
}
void teamsDialog::loadTeams()
{
    listTeams_->clear();
    auto res = logTeam_->list();
    for (const auto& t : res.data) {
        auto tr = logTournament_->obtain(t.tournamentId);
        QString tourName = (!tr.data.empty())
            ? QString::fromStdString(tr.data.front().name) : "Sin torneo";
        QListWidgetItem* item = new QListWidgetItem(
            QString("%1  —  %2  (%3 pts)")
            .arg(QString::fromStdString(t.name), tourName).arg(t.points),
            listTeams_);
        item->setData(Qt::UserRole, t.id);
        item->setData(Qt::UserRole + 1, t.tournamentId);
        listTeams_->addItem(item);
    }
    if (lblSubtitle_) lblSubtitle_->setText(QString("%1 equipo(s)").arg(res.data.size()));
}
void teamsDialog::onAddClicked()
{
    QString n = edtName_->text().trimmed();
    int tId = cmbTournament_->currentData().toInt();
    if (n.isEmpty() || tId < 0) { edtName_->setFocus(); return; }
    auto r = logTeam_->insert(n.toStdString(), tId);
    if (r.code >= 0) { edtName_->clear(); loadTeams(); }
    else QMessageBox::warning(this, "Error", QString::fromStdString(r.message));
}
void teamsDialog::onEditClicked()
{
    auto* item = listTeams_->currentItem(); if (!item) return;
    bool ok;
    QString nn = QInputDialog::getText(this, "Editar equipo", "Nuevo nombre:",
        QLineEdit::Normal, item->text().split("  —  ").first(), &ok);
    if (ok && !nn.trimmed().isEmpty()) {
        auto r = logTeam_->update(item->data(Qt::UserRole).toInt(), nn.trimmed().toStdString());
        if (r.code >= 0) loadTeams();
        else QMessageBox::warning(this, "Error", QString::fromStdString(r.message));
    }
}
void teamsDialog::onDeleteClicked()
{
    auto* item = listTeams_->currentItem(); if (!item) return;
    if (QMessageBox::warning(this, "Eliminar equipo", "¿Eliminar este equipo?",
        QMessageBox::Yes | QMessageBox::Cancel) == QMessageBox::Yes) {
        auto r = logTeam_->eliminar(item->data(Qt::UserRole).toInt());
        if (r.code >= 0) loadTeams();
        else QMessageBox::warning(this, "Error", QString::fromStdString(r.message));
    }
}
void teamsDialog::onRefresh() { loadTournaments(); loadTeams(); }
