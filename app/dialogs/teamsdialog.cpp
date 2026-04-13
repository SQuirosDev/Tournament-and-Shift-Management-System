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

static const char* SS_LIST =
"QListWidget { border:1px solid #e6e6ea; border-radius:12px;"
"  background:#ffffff; padding:6px; outline:none; color:#1d1d1f; }"
"QListWidget::item { border-radius:8px; padding:10px 12px;"
"  margin:1px 0; color:#1d1d1f; }"
"QListWidget::item:selected { background:#e8f2ff; color:#0071e3; }"
"QListWidget::item:hover:!selected { background:#f5f5f7; color:#1d1d1f; }";
static const char* SS_EDIT =
"QLineEdit { background:#fbfbfd; border:1px solid #e6e6ea; border-radius:10px;"
"  padding:9px 13px; font-size:13px; color:#1d1d1f; }"
"QLineEdit:focus { border:2px solid #0071e3; background:white; }";
static const char* SS_COMBO =
"QComboBox { background:#fbfbfd; border:1px solid #e6e6ea; border-radius:10px;"
"  padding:8px 13px; font-size:13px; color:#1d1d1f; }"
"QComboBox::drop-down { border:none; width:24px; }";
static const char* SS_PRIMARY =
"QPushButton { background:#0071e3; color:white; border:none; border-radius:9px;"
"  padding:8px 18px; font-size:13px; font-weight:600; }"
"QPushButton:hover { background:#0077ed; } QPushButton:disabled { background:#b3d4f5; }";
static const char* SS_SEC =
"QPushButton { background:#fbfbfd; color:#1d1d1f; border:1px solid #e6e6ea;"
"  border-radius:9px; padding:8px 18px; font-size:13px; font-weight:600; }"
"QPushButton:hover { background:#e8e8ed; } QPushButton:disabled { color:#b0b0b8; }";
static const char* SS_DANGER =
"QPushButton { background:#ff3b30; color:white; border:none; border-radius:9px;"
"  padding:8px 18px; font-size:13px; font-weight:600; }"
"QPushButton:hover { background:#ff453a; } QPushButton:disabled { background:#ffb3af; }";

teamsDialog::teamsDialog(LogTeam* logTeam, LogTournament* logTournament, QWidget* parent)
    : QDialog(parent), logTeam_(logTeam), logTournament_(logTournament)
{
    setWindowTitle("Gestionar Equipos");
    setMinimumSize(540, 440);
    resize(560, 460);
    setStyleSheet("QDialog { background:#f5f5f7; }");

    QVBoxLayout* root = new QVBoxLayout(this);
    root->setSpacing(0);
    root->setContentsMargins(0, 0, 0, 0);

    // Header
    QWidget* hdr = new QWidget();
    hdr->setStyleSheet("background:#ffffff; border-bottom:1px solid #e6e6ea;");
    QHBoxLayout* hdrL = new QHBoxLayout(hdr);
    hdrL->setContentsMargins(24, 16, 24, 16);
    hdrL->setSpacing(12);
    QLabel* ico = new QLabel("E");
    ico->setFixedSize(40, 40);
    ico->setAlignment(Qt::AlignCenter);
    ico->setStyleSheet("background:rgba(0,113,227,0.12); border-radius:10px;"
        "font-size:16px; font-weight:700; color:#0071e3;");
    QVBoxLayout* titleCol = new QVBoxLayout();
    titleCol->setSpacing(1);
    QLabel* lTitle = new QLabel("Equipos");
    lTitle->setStyleSheet("font-size:15px; font-weight:700; color:#1d1d1f;");
    lblSubtitle_ = new QLabel();
    lblSubtitle_->setStyleSheet("font-size:11px; color:#86868b;");
    titleCol->addWidget(lTitle);
    titleCol->addWidget(lblSubtitle_);
    hdrL->addWidget(ico);
    hdrL->addLayout(titleCol);
    hdrL->addStretch();
    root->addWidget(hdr);

    // Body
    QWidget* body = new QWidget();
    body->setStyleSheet("background:#f5f5f7;");
    QVBoxLayout* bodyL = new QVBoxLayout(body);
    bodyL->setContentsMargins(20, 14, 20, 14);
    bodyL->setSpacing(10);

    // Barra agregar
    QWidget* addBar = new QWidget();
    addBar->setStyleSheet("background:#ffffff; border:1px solid #e6e6ea; border-radius:12px;");
    QHBoxLayout* addL = new QHBoxLayout(addBar);
    addL->setContentsMargins(12, 8, 12, 8);
    addL->setSpacing(8);
    edtName_ = new QLineEdit();
    edtName_->setPlaceholderText("Nombre del equipo...");
    edtName_->setStyleSheet(SS_EDIT);
    cmbTournament_ = new QComboBox();
    cmbTournament_->setMinimumWidth(150);
    cmbTournament_->setStyleSheet(SS_COMBO);
    QPushButton* btnAdd = new QPushButton("Agregar");
    btnAdd->setStyleSheet(SS_PRIMARY);
    btnAdd->setFixedHeight(36);
    addL->addWidget(edtName_, 1);
    addL->addWidget(cmbTournament_);
    addL->addWidget(btnAdd);
    bodyL->addWidget(addBar);

    // Lista
    listTeams_ = new QListWidget();
    listTeams_->setStyleSheet(SS_LIST);
    listTeams_->setSelectionMode(QAbstractItemView::SingleSelection);
    bodyL->addWidget(listTeams_, 1);

    // Toolbar
    QHBoxLayout* toolL = new QHBoxLayout();
    toolL->setSpacing(8);
    btnEdit_ = new QPushButton("Editar nombre");
    btnDelete_ = new QPushButton("Eliminar");
    QPushButton* btnRefresh = new QPushButton("Actualizar");
    btnEdit_->setStyleSheet(SS_SEC);
    btnDelete_->setStyleSheet(SS_DANGER);
    btnRefresh->setStyleSheet(SS_SEC);
    btnEdit_->setEnabled(false);
    btnDelete_->setEnabled(false);
    toolL->addWidget(btnEdit_);
    toolL->addWidget(btnDelete_);
    toolL->addStretch();
    toolL->addWidget(btnRefresh);
    bodyL->addLayout(toolL);

    root->addWidget(body, 1);

    connect(btnAdd, &QPushButton::clicked, this, &teamsDialog::onAddClicked);
    connect(btnEdit_, &QPushButton::clicked, this, &teamsDialog::onEditClicked);
    connect(btnDelete_, &QPushButton::clicked, this, &teamsDialog::onDeleteClicked);
    connect(btnRefresh, &QPushButton::clicked, this, &teamsDialog::onRefresh);
    connect(edtName_, &QLineEdit::returnPressed, this, &teamsDialog::onAddClicked);
    connect(listTeams_, &QListWidget::itemSelectionChanged, this, [this]() {
        bool sel = !listTeams_->selectedItems().isEmpty();
        btnEdit_->setEnabled(sel);
        btnDelete_->setEnabled(sel);
        });

    loadTournaments();
    loadTeams();
}

teamsDialog::~teamsDialog() {}

void teamsDialog::loadTournaments()
{
    cmbTournament_->clear();
    cmbTournament_->addItem("— Torneo —", -1);
    auto res = logTournament_->list();
    for (const auto& t : res.data)
        cmbTournament_->addItem(QString::fromStdString(t.name), t.id);
}

void teamsDialog::loadTeams()
{
    listTeams_->clear();
    auto res = logTeam_->list();
    for (const auto& t : res.data) {
        // Obtener nombre del torneo para mostrar contexto
        auto tourRes = logTournament_->obtain(t.tournamentId);
        QString tourName = (!tourRes.data.empty())
            ? QString::fromStdString(tourRes.data.front().name)
            : "Sin torneo";
        QString text = QString("%1  —  %2  (%3 pts)")
            .arg(QString::fromStdString(t.name), tourName)
            .arg(t.points);
        QListWidgetItem* item = new QListWidgetItem(text, listTeams_);
        item->setData(Qt::UserRole, t.id);
        item->setData(Qt::UserRole + 1, t.tournamentId);
        listTeams_->addItem(item);
    }
    if (lblSubtitle_)
        lblSubtitle_->setText(QString("%1 equipo(s)").arg(res.data.size()));
}

void teamsDialog::onAddClicked()
{
    QString name = edtName_->text().trimmed();
    int tId = cmbTournament_->currentData().toInt();
    if (name.isEmpty() || tId < 0) { edtName_->setFocus(); return; }
    auto res = logTeam_->insert(name.toStdString(), tId);
    if (res.code >= 0) { edtName_->clear(); loadTeams(); }
    else QMessageBox::warning(this, "Error", QString::fromStdString(res.message));
}

void teamsDialog::onEditClicked()
{
    QListWidgetItem* item = listTeams_->currentItem();
    if (!item) return;
    QString current = item->text().split("  —  ").first();
    bool ok;
    QString newName = QInputDialog::getText(this, "Editar equipo",
        "Nuevo nombre:", QLineEdit::Normal, current, &ok);
    if (ok && !newName.trimmed().isEmpty()) {
        auto res = logTeam_->update(item->data(Qt::UserRole).toInt(),
            newName.trimmed().toStdString());
        if (res.code >= 0) loadTeams();
        else QMessageBox::warning(this, "Error", QString::fromStdString(res.message));
    }
}

void teamsDialog::onDeleteClicked()
{
    QListWidgetItem* item = listTeams_->currentItem();
    if (!item) return;
    if (QMessageBox::warning(this, "Eliminar equipo",
        "¿Eliminar este equipo?",
        QMessageBox::Yes | QMessageBox::Cancel) == QMessageBox::Yes) {
        auto res = logTeam_->eliminar(item->data(Qt::UserRole).toInt());
        if (res.code >= 0) loadTeams();
        else QMessageBox::warning(this, "Error", QString::fromStdString(res.message));
    }
}

void teamsDialog::onRefresh() { loadTournaments(); loadTeams(); }
