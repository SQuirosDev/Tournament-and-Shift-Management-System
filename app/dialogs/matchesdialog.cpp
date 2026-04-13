#include "dialogs/matchesdialog.h"
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
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
static const char* SS_SUCCESS =
"QPushButton { background:#34c759; color:white; border:none; border-radius:9px;"
"  padding:8px 18px; font-size:13px; font-weight:600; }"
"QPushButton:hover { background:#30d158; } QPushButton:disabled { background:#a8e6b8; }";

matchesDialog::matchesDialog(LogMatch* logMatch, LogGame* logGame,
    LogTournament* logTournament, LogTeam* logTeam,
    QWidget* parent)
    : QDialog(parent)
    , logMatch_(logMatch), logGame_(logGame)
    , logTournament_(logTournament), logTeam_(logTeam)
{
    setWindowTitle("Gestionar Partidos");
    setMinimumSize(600, 480);
    resize(620, 500);
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
    QLabel* ico = new QLabel("P");
    ico->setFixedSize(40, 40);
    ico->setAlignment(Qt::AlignCenter);
    ico->setStyleSheet("background:rgba(0,113,227,0.12); border-radius:10px;"
        "font-size:16px; font-weight:700; color:#0071e3;");
    QVBoxLayout* titleCol = new QVBoxLayout();
    titleCol->setSpacing(1);
    QLabel* lTitle = new QLabel("Partidos");
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

    // Selector de torneo
    QWidget* filterBar = new QWidget();
    filterBar->setStyleSheet("background:#ffffff; border:1px solid #e6e6ea; border-radius:12px;");
    QHBoxLayout* filterL = new QHBoxLayout(filterBar);
    filterL->setContentsMargins(12, 8, 12, 8);
    filterL->setSpacing(8);
    QLabel* lTour = new QLabel("Torneo:");
    lTour->setStyleSheet("font-size:12px; font-weight:600; color:#86868b;");
    cmbTournament_ = new QComboBox();
    cmbTournament_->setStyleSheet(SS_COMBO);
    filterL->addWidget(lTour);
    filterL->addWidget(cmbTournament_, 1);
    bodyL->addWidget(filterBar);

    // Barra agregar partido manual
    QWidget* addBar = new QWidget();
    addBar->setStyleSheet("background:#ffffff; border:1px solid #e6e6ea; border-radius:12px;");
    QHBoxLayout* addL = new QHBoxLayout(addBar);
    addL->setContentsMargins(12, 8, 12, 8);
    addL->setSpacing(8);
    cmbTeamA_ = new QComboBox();
    cmbTeamA_->setStyleSheet(SS_COMBO);
    QLabel* lVs = new QLabel("vs");
    lVs->setAlignment(Qt::AlignCenter);
    lVs->setStyleSheet("font-size:12px; font-weight:700; color:#86868b;");
    cmbTeamB_ = new QComboBox();
    cmbTeamB_->setStyleSheet(SS_COMBO);
    QPushButton* btnAdd = new QPushButton("Agregar partido");
    btnAdd->setStyleSheet(SS_PRIMARY);
    btnAdd->setFixedHeight(36);
    addL->addWidget(cmbTeamA_, 1);
    addL->addWidget(lVs);
    addL->addWidget(cmbTeamB_, 1);
    addL->addWidget(btnAdd);
    bodyL->addWidget(addBar);

    // Acciones rápidas de simulación
    QWidget* simBar = new QWidget();
    simBar->setStyleSheet("background:#ffffff; border:1px solid #e6e6ea; border-radius:12px;");
    QHBoxLayout* simL = new QHBoxLayout(simBar);
    simL->setContentsMargins(12, 8, 12, 8);
    simL->setSpacing(8);
    QLabel* lSim = new QLabel("Simular:");
    lSim->setStyleSheet("font-size:12px; font-weight:600; color:#86868b;");
    btnGenGroups_ = new QPushButton("Generar grupos");
    btnPlayGroups_ = new QPushButton("Jugar grupos");
    btnGenSemi_ = new QPushButton("Generar semi");
    btnPlaySemi_ = new QPushButton("Jugar semi");
    btnGenFinal_ = new QPushButton("Generar final");
    btnPlayFinal_ = new QPushButton("Jugar final");
    for (auto* b : { btnGenGroups_, btnPlayGroups_, btnGenSemi_,
                    btnPlaySemi_, btnGenFinal_, btnPlayFinal_ }) {
        b->setStyleSheet(SS_SEC);
        b->setFixedHeight(32);
        simL->addWidget(b);
    }
    simL->insertWidget(0, lSim);
    bodyL->addWidget(simBar);

    // Lista
    listMatches_ = new QListWidget();
    listMatches_->setStyleSheet(SS_LIST);
    listMatches_->setSelectionMode(QAbstractItemView::SingleSelection);
    bodyL->addWidget(listMatches_, 1);

    // Toolbar
    QHBoxLayout* toolL = new QHBoxLayout();
    toolL->setSpacing(8);
    btnMarkPlayed_ = new QPushButton("Marcar jugado");
    btnEditResult_ = new QPushButton("Editar resultado");
    btnDeleteMatch_ = new QPushButton("Eliminar");
    QPushButton* btnRefresh = new QPushButton("Actualizar");
    btnMarkPlayed_->setStyleSheet(SS_SUCCESS);
    btnEditResult_->setStyleSheet(SS_SEC);
    btnDeleteMatch_->setStyleSheet(SS_DANGER);
    btnRefresh->setStyleSheet(SS_SEC);
    btnMarkPlayed_->setEnabled(false);
    btnEditResult_->setEnabled(false);
    btnDeleteMatch_->setEnabled(false);
    toolL->addWidget(btnMarkPlayed_);
    toolL->addWidget(btnEditResult_);
    toolL->addWidget(btnDeleteMatch_);
    toolL->addStretch();
    toolL->addWidget(btnRefresh);
    bodyL->addLayout(toolL);

    root->addWidget(body, 1);

    // ── Conexiones ────────────────────────────────────────────────────────────
    connect(btnAdd, &QPushButton::clicked, this, &matchesDialog::onAddClicked);
    connect(btnRefresh, &QPushButton::clicked, this, &matchesDialog::onRefresh);
    connect(btnDeleteMatch_, &QPushButton::clicked, this, &matchesDialog::onDeleteClicked);

    connect(btnMarkPlayed_, &QPushButton::clicked, this, [this]() {
        QListWidgetItem* item = listMatches_->currentItem();
        if (!item) return;
        int id = item->data(Qt::UserRole).toInt();
        bool ok;
        QString result = QInputDialog::getText(this, "Resultado",
            "Ingresa el resultado (ej. 2-1):", QLineEdit::Normal, "0-0", &ok);
        if (!ok || result.trimmed().isEmpty()) return;
        // Determinar ganador por el marcador
        QStringList parts = result.split("-");
        int winnerId = 0;
        if (parts.size() == 2) {
            int sA = parts[0].trimmed().toInt();
            int sB = parts[1].trimmed().toInt();
            int teamAId = item->data(Qt::UserRole + 1).toInt();
            int teamBId = item->data(Qt::UserRole + 2).toInt();
            if (sA > sB) winnerId = teamAId;
            else if (sB > sA) winnerId = teamBId;
        }
        auto res = logMatch_->update(id, "Grupos", 0, "Finalizado",
            winnerId, result.toStdString());
        if (res.code >= 0) loadMatches(cmbTournament_->currentData().toInt());
        else QMessageBox::warning(this, "Error", QString::fromStdString(res.message));
        });

    connect(btnEditResult_, &QPushButton::clicked, this, [this]() {
        QListWidgetItem* item = listMatches_->currentItem();
        if (!item) return;
        int id = item->data(Qt::UserRole).toInt();
        bool ok;
        QString result = QInputDialog::getText(this, "Editar resultado",
            "Nuevo resultado:", QLineEdit::Normal,
            item->data(Qt::UserRole + 3).toString(), &ok);
        if (!ok) return;
        int winnerId = item->data(Qt::UserRole + 4).toInt();
        auto res = logMatch_->update(id, "Grupos", 0, "Finalizado",
            winnerId, result.toStdString());
        if (res.code >= 0) loadMatches(cmbTournament_->currentData().toInt());
        else QMessageBox::warning(this, "Error", QString::fromStdString(res.message));
        });

    connect(listMatches_, &QListWidget::itemSelectionChanged, this, [this]() {
        bool sel = !listMatches_->selectedItems().isEmpty();
        btnMarkPlayed_->setEnabled(sel);
        btnEditResult_->setEnabled(sel);
        btnDeleteMatch_->setEnabled(sel);
        });

    connect(cmbTournament_, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, [this](int) {
            int tId = cmbTournament_->currentData().toInt();
            if (tId > 0) { loadTeamsForTournament(tId); loadMatches(tId); }
        });

    // Simulación LogGame
    connect(btnGenGroups_, &QPushButton::clicked, this, [this]() { runGame([this](int id) { return logGame_->generateGroupMatches(id); }); });
    connect(btnPlayGroups_, &QPushButton::clicked, this, [this]() { runGame([this](int id) { return logGame_->playGroupMatches(id); }); });
    connect(btnGenSemi_, &QPushButton::clicked, this, [this]() { runGame([this](int id) { return logGame_->generateSemiMatches(id); }); });
    connect(btnPlaySemi_, &QPushButton::clicked, this, [this]() { runGame([this](int id) { return logGame_->playSemiMatches(id); }); });
    connect(btnGenFinal_, &QPushButton::clicked, this, [this]() { runGame([this](int id) { return logGame_->generateFinalMatch(id); }); });
    connect(btnPlayFinal_, &QPushButton::clicked, this, [this]() { runGame([this](int id) { return logGame_->playFinalMatch(id); }); });

    loadTournaments();
}

matchesDialog::~matchesDialog() {}

// Helper: ejecuta una acción de LogGame sobre el torneo seleccionado
void matchesDialog::runGame(std::function<BackendResponse(int)> action)
{
    int tId = cmbTournament_->currentData().toInt();
    if (tId <= 0) {
        QMessageBox::warning(this, "Aviso", "Selecciona un torneo primero.");
        return;
    }
    auto res = action(tId);
    QString msg = QString::fromStdString(res.message);
    if (res.code >= 0) {
        loadMatches(tId);
        QMessageBox::information(this, "Listo", msg);
    }
    else {
        QMessageBox::warning(this, "Error", msg);
    }
}

void matchesDialog::loadTournaments()
{
    cmbTournament_->clear();
    cmbTournament_->addItem("— Selecciona un torneo —", -1);
    auto res = logTournament_->list();
    for (const auto& t : res.data)
        cmbTournament_->addItem(QString::fromStdString(t.name), t.id);
}

void matchesDialog::loadTeamsForTournament(int tournamentId)
{
    cmbTeamA_->clear();
    cmbTeamB_->clear();
    cmbTeamA_->addItem("— Equipo A —", -1);
    cmbTeamB_->addItem("— Equipo B —", -1);
    // Usamos logTournament para obtener equipos del torneo via connection directa
    // LogTeam::list() trae todos; filtramos por tournamentId
    auto res = logTeam_->list();
    for (const auto& t : res.data) {
        if (t.tournamentId != tournamentId) continue;
        QString name = QString::fromStdString(t.name);
        cmbTeamA_->addItem(name, t.id);
        cmbTeamB_->addItem(name, t.id);
    }
}

void matchesDialog::loadMatches(int tournamentId)
{
    listMatches_->clear();
    auto res = logMatch_->listByTournament(tournamentId);
    for (const auto& m : res.data) {
        // Nombres de equipos
        QString teamA, teamB;
        auto allTeams = logTeam_->list();
        for (const auto& t : allTeams.data) {
            if (t.id == m.teamAId) teamA = QString::fromStdString(t.name);
            if (t.id == m.teamBId) teamB = QString::fromStdString(t.name);
        }
        QString status = QString::fromStdString(m.status);
        QString result = QString::fromStdString(m.result.empty() ? "Pendiente" : m.result);
        QString phase = QString::fromStdString(m.phase);
        QString prefix = (status == "Finalizado") ? "[OK]" : "[--]";

        QString text = QString("%1  %2  vs  %3  —  %4  (%5, R%6)")
            .arg(prefix, teamA, teamB, result, phase).arg(m.round);

        QListWidgetItem* item = new QListWidgetItem(text, listMatches_);
        item->setData(Qt::UserRole, m.id);
        item->setData(Qt::UserRole + 1, m.teamAId);
        item->setData(Qt::UserRole + 2, m.teamBId);
        item->setData(Qt::UserRole + 3, result);
        item->setData(Qt::UserRole + 4, m.winnerId);
        listMatches_->addItem(item);
    }
    if (lblSubtitle_)
        lblSubtitle_->setText(QString("%1 partido(s)").arg(res.data.size()));
}

void matchesDialog::onAddClicked()
{
    int tId = cmbTournament_->currentData().toInt();
    int teamA = cmbTeamA_->currentData().toInt();
    int teamB = cmbTeamB_->currentData().toInt();
    if (tId < 0 || teamA < 0 || teamB < 0 || teamA == teamB) {
        QMessageBox::warning(this, "Datos incompletos",
            "Selecciona un torneo y dos equipos distintos.");
        return;
    }
    auto res = logMatch_->insert(tId, teamA, teamB);
    if (res.code >= 0) loadMatches(tId);
    else QMessageBox::warning(this, "Error", QString::fromStdString(res.message));
}

void matchesDialog::onDeleteClicked()
{
    QListWidgetItem* item = listMatches_->currentItem();
    if (!item) return;
    if (QMessageBox::warning(this, "Eliminar partido",
        "¿Eliminar este partido?",
        QMessageBox::Yes | QMessageBox::Cancel) == QMessageBox::Yes) {
        auto res = logMatch_->eliminar(item->data(Qt::UserRole).toInt());
        if (res.code >= 0) loadMatches(cmbTournament_->currentData().toInt());
        else QMessageBox::warning(this, "Error", QString::fromStdString(res.message));
    }
}

void matchesDialog::onEditClicked() {}  // Manejado por lambda de btnEditResult_

void matchesDialog::onRefresh()
{
    loadTournaments();
    int tId = cmbTournament_->currentData().toInt();
    if (tId > 0) { loadTeamsForTournament(tId); loadMatches(tId); }
}
