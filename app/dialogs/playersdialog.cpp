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

static const char* SS_HEADER = "background:#ffffff;border-bottom:1px solid #e0e0e8;";
static const char* SS_BODY = "background:#f5f5f7;";
static const char* SS_BAR = "background:#ffffff;border:1px solid #e0e0e8;border-radius:12px;";
static const char* SS_ICO = "background:rgba(0,113,227,0.10);border-radius:10px;font-size:15px;font-weight:700;color:#0071e3;";
static const char* SS_PRIMARY = "QPushButton{background:#0071e3;color:#fff;border:none;border-radius:9px;padding:8px 18px;font-size:13px;font-weight:600;}QPushButton:hover{background:#0077ed;}QPushButton:disabled{background:#b3d4f5;}";
static const char* SS_SEC = "QPushButton{background:#ffffff;color:#1d1d1f;border:1px solid #d2d2d7;border-radius:9px;padding:8px 18px;font-size:13px;font-weight:600;}QPushButton:hover{background:#f0f0f5;}QPushButton:disabled{color:#b0b0b8;}";
static const char* SS_DANGER = "QPushButton{background:#ff3b30;color:#fff;border:none;border-radius:9px;padding:8px 18px;font-size:13px;font-weight:600;}QPushButton:hover{background:#ff453a;}QPushButton:disabled{background:#ffb3af;}";

playersDialog::playersDialog(LogPlayer* logPlayer, LogTeam* logTeam, QWidget* parent)
    : QDialog(parent), logPlayer_(logPlayer), logTeam_(logTeam)
{
    setWindowTitle("Gestionar Jugadores");
    setFixedSize(580, 500);

    QVBoxLayout* root = new QVBoxLayout(this);
    root->setSpacing(0); root->setContentsMargins(0, 0, 0, 0);

    // Header
    QWidget* hdr = new QWidget(); hdr->setStyleSheet(SS_HEADER);
    QHBoxLayout* hL = new QHBoxLayout(hdr);
    hL->setContentsMargins(22, 14, 22, 14); hL->setSpacing(12);
    QLabel* ico = new QLabel("J"); ico->setFixedSize(38, 38);
    ico->setAlignment(Qt::AlignCenter); ico->setStyleSheet(SS_ICO);
    QVBoxLayout* tc = new QVBoxLayout(); tc->setSpacing(1);
    QLabel* lt = new QLabel("Jugadores");
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

    // Filtro equipo
    QWidget* filterBar = new QWidget(); filterBar->setStyleSheet(SS_BAR);
    QHBoxLayout* fL = new QHBoxLayout(filterBar);
    fL->setContentsMargins(12, 8, 12, 8); fL->setSpacing(8);
    QLabel* lF = new QLabel("Filtrar por equipo:");
    lF->setStyleSheet("font-size:12px;font-weight:600;color:#86868b;");
    cmbFilter_ = new QComboBox(); cmbFilter_->setMinimumWidth(200);
    fL->addWidget(lF); fL->addWidget(cmbFilter_, 1);
    bL->addWidget(filterBar);

    // Barra agregar
    QWidget* addBar = new QWidget(); addBar->setStyleSheet(SS_BAR);
    QHBoxLayout* aL = new QHBoxLayout(addBar);
    aL->setContentsMargins(12, 8, 12, 8); aL->setSpacing(8);
    edtName_ = new QLineEdit(); edtName_->setPlaceholderText("Nombre del jugador...");
    cmbTeam_ = new QComboBox(); cmbTeam_->setMinimumWidth(160);
    QPushButton* btnAdd = new QPushButton("Agregar");
    btnAdd->setStyleSheet(SS_PRIMARY); btnAdd->setFixedHeight(36);
    aL->addWidget(edtName_, 1); aL->addWidget(cmbTeam_); aL->addWidget(btnAdd);
    bL->addWidget(addBar);

    // Lista
    listPlayers_ = new QListWidget();
    listPlayers_->setSelectionMode(QAbstractItemView::SingleSelection);
    bL->addWidget(listPlayers_, 1);

    // Toolbar
    QHBoxLayout* tL = new QHBoxLayout(); tL->setSpacing(8);
    btnEdit_ = new QPushButton("Editar nombre"); btnEdit_->setStyleSheet(SS_SEC);
    btnDelete_ = new QPushButton("Eliminar");      btnDelete_->setStyleSheet(SS_DANGER);
    QPushButton* btnR = new QPushButton("Actualizar"); btnR->setStyleSheet(SS_SEC);
    btnEdit_->setEnabled(false); btnDelete_->setEnabled(false);
    tL->addWidget(btnEdit_); tL->addWidget(btnDelete_); tL->addStretch(); tL->addWidget(btnR);
    bL->addLayout(tL);
    root->addWidget(body, 1);

    connect(btnAdd, &QPushButton::clicked, this, &playersDialog::onAddClicked);
    connect(btnEdit_, &QPushButton::clicked, this, &playersDialog::onEditClicked);
    connect(btnDelete_, &QPushButton::clicked, this, &playersDialog::onDeleteClicked);
    connect(btnR, &QPushButton::clicked, this, &playersDialog::onRefresh);
    connect(edtName_, &QLineEdit::returnPressed, this, &playersDialog::onAddClicked);
    connect(cmbFilter_, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, [this](int) { loadPlayers(); });
    connect(listPlayers_, &QListWidget::itemSelectionChanged, this, [this]() {
        bool s = !listPlayers_->selectedItems().isEmpty();
        btnEdit_->setEnabled(s); btnDelete_->setEnabled(s);
        });

    loadTeams(); loadPlayers();
}
playersDialog::~playersDialog() {}

void playersDialog::loadTeams()
{
    cmbTeam_->clear();   cmbTeam_->addItem("— Equipo —", -1);
    cmbFilter_->clear(); cmbFilter_->addItem("Todos los equipos", -1);
    for (const auto& t : logTeam_->list().data) {
        QString n = QString::fromStdString(t.name);
        cmbTeam_->addItem(n, t.id);
        cmbFilter_->addItem(n, t.id);
    }
}
void playersDialog::loadPlayers()
{
    listPlayers_->clear();
    int filterId = cmbFilter_->currentData().toInt();
    int total = 0;
    auto teams = logTeam_->list();
    for (const auto& t : teams.data) {
        if (filterId > 0 && t.id != filterId) continue;
        QString teamName = QString::fromStdString(t.name);
        for (const auto& p : logPlayer_->listByTeam(t.id).data) {
            QListWidgetItem* item = new QListWidgetItem(
                QString("%1  —  %2").arg(QString::fromStdString(p.name), teamName),
                listPlayers_);
            item->setData(Qt::UserRole, p.id);
            item->setData(Qt::UserRole + 1, p.teamId);
            listPlayers_->addItem(item);
            total++;
        }
    }
    if (lblSubtitle_) lblSubtitle_->setText(QString("%1 jugador(es)").arg(total));
}
void playersDialog::onAddClicked()
{
    QString n = edtName_->text().trimmed();
    int tId = cmbTeam_->currentData().toInt();
    if (n.isEmpty() || tId < 0) { edtName_->setFocus(); return; }
    auto r = logPlayer_->insert(tId, n.toStdString());
    if (r.code >= 0) { edtName_->clear(); loadPlayers(); }
    else QMessageBox::warning(this, "Error", QString::fromStdString(r.message));
}
void playersDialog::onEditClicked()
{
    auto* item = listPlayers_->currentItem(); if (!item) return;
    bool ok;
    QString nn = QInputDialog::getText(this, "Editar jugador", "Nuevo nombre:",
        QLineEdit::Normal, item->text().split("  —  ").first(), &ok);
    if (ok && !nn.trimmed().isEmpty()) {
        auto r = logPlayer_->update(item->data(Qt::UserRole).toInt(), nn.trimmed().toStdString());
        if (r.code >= 0) loadPlayers();
        else QMessageBox::warning(this, "Error", QString::fromStdString(r.message));
    }
}
void playersDialog::onDeleteClicked()
{
    auto* item = listPlayers_->currentItem(); if (!item) return;
    if (QMessageBox::warning(this, "Eliminar jugador", "¿Eliminar este jugador?",
        QMessageBox::Yes | QMessageBox::Cancel) == QMessageBox::Yes) {
        auto r = logPlayer_->eliminar(item->data(Qt::UserRole).toInt());
        if (r.code >= 0) loadPlayers();
        else QMessageBox::warning(this, "Error", QString::fromStdString(r.message));
    }
}
void playersDialog::onRefresh() { loadTeams(); loadPlayers(); }
