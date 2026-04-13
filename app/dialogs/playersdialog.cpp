#include "dialogs/playersdialog.h"
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

playersDialog::playersDialog(LogPlayer* logPlayer, LogTeam* logTeam, QWidget* parent)
    : QDialog(parent), logPlayer_(logPlayer), logTeam_(logTeam)
{
    setWindowTitle("Gestionar Jugadores");
    setMinimumSize(540, 460);
    resize(560, 480);
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
    QLabel* ico = new QLabel("J");
    ico->setFixedSize(40, 40);
    ico->setAlignment(Qt::AlignCenter);
    ico->setStyleSheet("background:rgba(0,113,227,0.12); border-radius:10px;"
        "font-size:16px; font-weight:700; color:#0071e3;");
    QVBoxLayout* titleCol = new QVBoxLayout();
    titleCol->setSpacing(1);
    QLabel* lTitle = new QLabel("Jugadores");
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

    // Filtro por equipo
    QWidget* filterBar = new QWidget();
    filterBar->setStyleSheet("background:#ffffff; border:1px solid #e6e6ea; border-radius:12px;");
    QHBoxLayout* filterL = new QHBoxLayout(filterBar);
    filterL->setContentsMargins(12, 8, 12, 8);
    filterL->setSpacing(8);
    QLabel* lFilter = new QLabel("Equipo:");
    lFilter->setStyleSheet("font-size:12px; font-weight:600; color:#86868b;");
    cmbFilter_ = new QComboBox();
    cmbFilter_->setStyleSheet(SS_COMBO);
    filterL->addWidget(lFilter);
    filterL->addWidget(cmbFilter_, 1);
    bodyL->addWidget(filterBar);

    // Barra agregar
    QWidget* addBar = new QWidget();
    addBar->setStyleSheet("background:#ffffff; border:1px solid #e6e6ea; border-radius:12px;");
    QHBoxLayout* addL = new QHBoxLayout(addBar);
    addL->setContentsMargins(12, 8, 12, 8);
    addL->setSpacing(8);
    edtName_ = new QLineEdit();
    edtName_->setPlaceholderText("Nombre del jugador...");
    edtName_->setStyleSheet(SS_EDIT);
    cmbTeam_ = new QComboBox();
    cmbTeam_->setMinimumWidth(150);
    cmbTeam_->setStyleSheet(SS_COMBO);
    QPushButton* btnAdd = new QPushButton("Agregar");
    btnAdd->setStyleSheet(SS_PRIMARY);
    btnAdd->setFixedHeight(36);
    addL->addWidget(edtName_, 1);
    addL->addWidget(cmbTeam_);
    addL->addWidget(btnAdd);
    bodyL->addWidget(addBar);

    // Lista
    listPlayers_ = new QListWidget();
    listPlayers_->setStyleSheet(SS_LIST);
    listPlayers_->setSelectionMode(QAbstractItemView::SingleSelection);
    bodyL->addWidget(listPlayers_, 1);

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

    connect(btnAdd, &QPushButton::clicked, this, &playersDialog::onAddClicked);
    connect(btnEdit_, &QPushButton::clicked, this, &playersDialog::onEditClicked);
    connect(btnDelete_, &QPushButton::clicked, this, &playersDialog::onDeleteClicked);
    connect(btnRefresh, &QPushButton::clicked, this, &playersDialog::onRefresh);
    connect(edtName_, &QLineEdit::returnPressed, this, &playersDialog::onAddClicked);
    connect(cmbFilter_, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, [this](int) { loadPlayers(); });
    connect(listPlayers_, &QListWidget::itemSelectionChanged, this, [this]() {
        bool sel = !listPlayers_->selectedItems().isEmpty();
        btnEdit_->setEnabled(sel);
        btnDelete_->setEnabled(sel);
        });

    loadTeams();
    loadPlayers();
}

playersDialog::~playersDialog() {}

void playersDialog::loadTeams()
{
    cmbTeam_->clear();
    cmbTeam_->addItem("— Equipo —", -1);
    cmbFilter_->clear();
    cmbFilter_->addItem("Todos los equipos", -1);
    auto res = logTeam_->list();
    for (const auto& t : res.data) {
        QString name = QString::fromStdString(t.name);
        cmbTeam_->addItem(name, t.id);
        cmbFilter_->addItem(name, t.id);
    }
}

void playersDialog::loadPlayers()
{
    listPlayers_->clear();
    int filterTeamId = cmbFilter_->currentData().toInt();

    // Si hay filtro activo usar listByTeam, si no listar todos team por team
    if (filterTeamId > 0) {
        auto res = logPlayer_->listByTeam(filterTeamId);
        auto teamRes = logTeam_->list();
        // Buscar nombre del equipo filtrado
        QString teamName;
        for (const auto& t : teamRes.data)
            if (t.id == filterTeamId) { teamName = QString::fromStdString(t.name); break; }
        for (const auto& p : res.data) {
            QListWidgetItem* item = new QListWidgetItem(
                QString("%1  —  %2").arg(QString::fromStdString(p.name), teamName),
                listPlayers_);
            item->setData(Qt::UserRole, p.id);
            item->setData(Qt::UserRole + 1, p.teamId);
            listPlayers_->addItem(item);
        }
        if (lblSubtitle_)
            lblSubtitle_->setText(QString("%1 jugador(es)").arg(res.data.size()));
    }
    else {
        // Cargar todos iterando equipos
        auto teamsRes = logTeam_->list();
        int total = 0;
        for (const auto& t : teamsRes.data) {
            auto res = logPlayer_->listByTeam(t.id);
            QString teamName = QString::fromStdString(t.name);
            for (const auto& p : res.data) {
                QListWidgetItem* item = new QListWidgetItem(
                    QString("%1  —  %2").arg(QString::fromStdString(p.name), teamName),
                    listPlayers_);
                item->setData(Qt::UserRole, p.id);
                item->setData(Qt::UserRole + 1, p.teamId);
                listPlayers_->addItem(item);
                total++;
            }
        }
        if (lblSubtitle_)
            lblSubtitle_->setText(QString("%1 jugador(es)").arg(total));
    }
}

void playersDialog::onAddClicked()
{
    QString name = edtName_->text().trimmed();
    int teamId = cmbTeam_->currentData().toInt();
    if (name.isEmpty() || teamId < 0) { edtName_->setFocus(); return; }
    auto res = logPlayer_->insert(teamId, name.toStdString());
    if (res.code >= 0) { edtName_->clear(); loadPlayers(); }
    else QMessageBox::warning(this, "Error", QString::fromStdString(res.message));
}

void playersDialog::onEditClicked()
{
    QListWidgetItem* item = listPlayers_->currentItem();
    if (!item) return;
    QString current = item->text().split("  —  ").first();
    bool ok;
    QString newName = QInputDialog::getText(this, "Editar jugador",
        "Nuevo nombre:", QLineEdit::Normal, current, &ok);
    if (ok && !newName.trimmed().isEmpty()) {
        auto res = logPlayer_->update(item->data(Qt::UserRole).toInt(),
            newName.trimmed().toStdString());
        if (res.code >= 0) loadPlayers();
        else QMessageBox::warning(this, "Error", QString::fromStdString(res.message));
    }
}

void playersDialog::onDeleteClicked()
{
    QListWidgetItem* item = listPlayers_->currentItem();
    if (!item) return;
    if (QMessageBox::warning(this, "Eliminar jugador",
        "¿Eliminar este jugador?",
        QMessageBox::Yes | QMessageBox::Cancel) == QMessageBox::Yes) {
        auto res = logPlayer_->eliminar(item->data(Qt::UserRole).toInt());
        if (res.code >= 0) loadPlayers();
        else QMessageBox::warning(this, "Error", QString::fromStdString(res.message));
    }
}

void playersDialog::onRefresh() { loadTeams(); loadPlayers(); }
