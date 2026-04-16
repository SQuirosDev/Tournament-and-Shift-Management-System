#include "dialogs/matchesdialog.h"

#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QTableWidgetItem>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QWidget>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QInputDialog>
#include <QtWidgets/QHeaderView>

static const char* SS_HEADER = "background:#ffffff;border-bottom:1px solid #e0e0e8;";
static const char* SS_BODY = "background:#f5f5f7;";
static const char* SS_BAR = "background:#ffffff;border:1px solid #e0e0e8;border-radius:12px;";
static const char* SS_ICO = "background:rgba(0,113,227,0.10);border-radius:10px;font-size:15px;font-weight:700;color:#0071e3;";
static const char* SS_PRIMARY = "QPushButton{background:#0071e3;color:#fff;border:none;border-radius:9px;padding:8px 18px;font-size:13px;font-weight:600;}QPushButton:hover{background:#0077ed;}QPushButton:disabled{background:#b3d4f5;}";
static const char* SS_SEC = "QPushButton{background:#ffffff;color:#1d1d1f;border:1px solid #d2d2d7;border-radius:9px;padding:8px 18px;font-size:13px;font-weight:600;}QPushButton:hover{background:#f0f0f5;}QPushButton:disabled{color:#b0b0b8;}";
static const char* SS_DANGER = "QPushButton{background:#ff3b30;color:#fff;border:none;border-radius:9px;padding:8px 18px;font-size:13px;font-weight:600;}QPushButton:hover{background:#ff453a;}QPushButton:disabled{background:#ffb3af;}";
static const char* SS_SUCCESS = "QPushButton{background:#34c759;color:#fff;border:none;border-radius:9px;padding:8px 18px;font-size:13px;font-weight:600;}QPushButton:hover{background:#30d158;}QPushButton:disabled{background:#a8e6b8;}";
static const char* SS_SIM = "QPushButton{background:#f5f5f7;color:#1d1d1f;border:1px solid #d2d2d7;border-radius:8px;padding:6px 12px;font-size:12px;font-weight:600;}QPushButton:hover{background:#e8e8ed;}QPushButton:disabled{color:#b0b0b8;}";

matchesDialog::matchesDialog(LogMatch* logMatch, LogGame* logGame,
    LogTournament* logTournament, LogTeam* logTeam,
    QWidget* parent)
    : QDialog(parent)
    , logMatch_(logMatch), logGame_(logGame)
    , logTournament_(logTournament), logTeam_(logTeam)
{
    setWindowTitle("Gestionar Partidos");
    setFixedSize(720, 600);

    QVBoxLayout* root = new QVBoxLayout(this);
    root->setSpacing(0); root->setContentsMargins(0, 0, 0, 0);

    // ── Header ────────────────────────────────────────────────────────────────
    QWidget* hdr = new QWidget(); hdr->setStyleSheet(SS_HEADER);
    QHBoxLayout* hL = new QHBoxLayout(hdr);
    hL->setContentsMargins(22, 14, 22, 14); hL->setSpacing(12);
    QLabel* ico = new QLabel("P"); ico->setFixedSize(38, 38);
    ico->setAlignment(Qt::AlignCenter); ico->setStyleSheet(SS_ICO);
    QVBoxLayout* tc = new QVBoxLayout(); tc->setSpacing(1);
    QLabel* lt = new QLabel("Partidos");
    lt->setStyleSheet("font-size:14px;font-weight:700;color:#1d1d1f;");
    lblSubtitle_ = new QLabel();
    lblSubtitle_->setStyleSheet("font-size:11px;color:#86868b;");
    tc->addWidget(lt); tc->addWidget(lblSubtitle_);
    hL->addWidget(ico); hL->addLayout(tc); hL->addStretch();
    root->addWidget(hdr);

    // ── Body ─────────────────────────────────────────────────────────────────
    QWidget* body = new QWidget(); body->setStyleSheet(SS_BODY);
    QVBoxLayout* bL = new QVBoxLayout(body);
    bL->setContentsMargins(18, 14, 18, 14); bL->setSpacing(10);

    // Selector de torneo
    QWidget* tourBar = new QWidget(); tourBar->setStyleSheet(SS_BAR);
    QHBoxLayout* tourL = new QHBoxLayout(tourBar);
    tourL->setContentsMargins(12, 8, 12, 8); tourL->setSpacing(8);
    QLabel* lT = new QLabel("Torneo:");
    lT->setStyleSheet("font-size:12px;font-weight:600;color:#86868b;");
    cmbTournament_ = new QComboBox(); cmbTournament_->setMinimumWidth(220);
    tourL->addWidget(lT); tourL->addWidget(cmbTournament_, 1);
    bL->addWidget(tourBar);

    // Barra agregar partido manual
    QWidget* addBar = new QWidget(); addBar->setStyleSheet(SS_BAR);
    QHBoxLayout* aL = new QHBoxLayout(addBar);
    aL->setContentsMargins(12, 8, 12, 8); aL->setSpacing(8);
    cmbTeamA_ = new QComboBox(); cmbTeamA_->setMinimumWidth(160);
    QLabel* lVs = new QLabel("vs");
    lVs->setAlignment(Qt::AlignCenter);
    lVs->setStyleSheet("font-size:12px;font-weight:700;color:#86868b;");
    cmbTeamB_ = new QComboBox(); cmbTeamB_->setMinimumWidth(160);
    QPushButton* btnAdd = new QPushButton("Agregar partido");
    btnAdd->setStyleSheet(SS_PRIMARY); btnAdd->setFixedHeight(36);
    aL->addWidget(cmbTeamA_, 1); aL->addWidget(lVs);
    aL->addWidget(cmbTeamB_, 1); aL->addWidget(btnAdd);
    bL->addWidget(addBar);

    // ── Simulación en 2 filas ─────────────────────────────────────────────────
    QWidget* simWidget = new QWidget(); simWidget->setStyleSheet(SS_BAR);
    QVBoxLayout* simV = new QVBoxLayout(simWidget);
    simV->setContentsMargins(12, 8, 12, 8); simV->setSpacing(6);
    QLabel* lSim = new QLabel("Simulación de fases:");
    lSim->setStyleSheet("font-size:12px;font-weight:600;color:#86868b;");
    simV->addWidget(lSim);
    QHBoxLayout* simR1 = new QHBoxLayout(); simR1->setSpacing(6);
    QHBoxLayout* simR2 = new QHBoxLayout(); simR2->setSpacing(6);
    btnGenGroups_ = new QPushButton("Generar grupos");
    btnPlayGroups_ = new QPushButton("Jugar grupos");
    btnGenSemi_ = new QPushButton("Generar semifinales");
    btnPlaySemi_ = new QPushButton("Jugar semifinales");
    btnGenFinal_ = new QPushButton("Generar final");
    btnPlayFinal_ = new QPushButton("Jugar final");
    for (auto* b : { btnGenGroups_, btnPlayGroups_, btnGenSemi_ }) {
        b->setStyleSheet(SS_SIM); simR1->addWidget(b);
    }
    simR1->addStretch();
    for (auto* b : { btnPlaySemi_, btnGenFinal_, btnPlayFinal_ }) {
        b->setStyleSheet(SS_SIM); simR2->addWidget(b);
    }
    simR2->addStretch();
    simV->addLayout(simR1); simV->addLayout(simR2);
    bL->addWidget(simWidget);

    // ──  Tabla de posiciones ─────────────────────────────────
    QTabWidget* tabs = new QTabWidget();
    tabs->setDocumentMode(false);

    // Tab 1 — lista de partidos
    QWidget* tabMatches = new QWidget();
    QVBoxLayout* tmL = new QVBoxLayout(tabMatches);
    tmL->setContentsMargins(0, 8, 0, 0); tmL->setSpacing(8);
    listMatches_ = new QListWidget();
    listMatches_->setSelectionMode(QAbstractItemView::SingleSelection);
    tmL->addWidget(listMatches_, 1);
    // Toolbar partidos
    QHBoxLayout* tL = new QHBoxLayout(); tL->setSpacing(8);
    btnMarkPlayed_ = new QPushButton("Marcar jugado");
    btnEditResult_ = new QPushButton("Editar resultado");
    btnDeleteMatch_ = new QPushButton("Eliminar");
    QPushButton* btnR = new QPushButton("Actualizar");
    btnMarkPlayed_->setStyleSheet(SS_SUCCESS);
    btnEditResult_->setStyleSheet(SS_SEC);
    btnDeleteMatch_->setStyleSheet(SS_DANGER);
    btnR->setStyleSheet(SS_SEC);
    btnMarkPlayed_->setEnabled(false);
    btnEditResult_->setEnabled(false);
    btnDeleteMatch_->setEnabled(false);
    tL->addWidget(btnMarkPlayed_); tL->addWidget(btnEditResult_);
    tL->addWidget(btnDeleteMatch_); tL->addStretch(); tL->addWidget(btnR);
    tmL->addLayout(tL);
    tabs->addTab(tabMatches, "Partidos");

    // Tab 2 — tabla de posiciones
    QWidget* tabStand = new QWidget();
    QVBoxLayout* tsL = new QVBoxLayout(tabStand);
    tsL->setContentsMargins(0, 8, 0, 0); tsL->setSpacing(6);
    tableStandings_ = new QTableWidget(0, 6);
    tableStandings_->setHorizontalHeaderLabels(
        { "Equipo","Pts","G","E","P","Torneo" });
    tableStandings_->horizontalHeader()->setStretchLastSection(true);
    tableStandings_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    tableStandings_->verticalHeader()->setVisible(false);
    tableStandings_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableStandings_->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableStandings_->setAlternatingRowColors(true);
    tableStandings_->setStyleSheet(
        "QTableWidget{alternate-background-color:#f8f8fb;}");
    QPushButton* btnRefreshStandings = new QPushButton("Actualizar tabla");
    btnRefreshStandings->setStyleSheet(SS_SEC);
    tsL->addWidget(tableStandings_, 1);
    tsL->addWidget(btnRefreshStandings, 0, Qt::AlignRight);
    tabs->addTab(tabStand, "Tabla de posiciones");

    bL->addWidget(tabs, 1);
    root->addWidget(body, 1);

    // ── Conexiones ────────────────────────────────────────────────────────────
    connect(btnAdd, &QPushButton::clicked, this, &matchesDialog::onAddClicked);
    connect(btnR, &QPushButton::clicked, this, &matchesDialog::onRefresh);
    connect(btnDeleteMatch_, &QPushButton::clicked, this, &matchesDialog::onDeleteClicked);

    connect(btnMarkPlayed_, &QPushButton::clicked, this, [this]() {
        auto* item = listMatches_->currentItem(); if (!item) return;
        bool ok;
        QString res = QInputDialog::getText(this, "Resultado",
            "Ingresa el resultado (ej. 2-1):", QLineEdit::Normal, "0-0", &ok);
        if (!ok || res.trimmed().isEmpty()) return;
        QStringList p = res.split("-");
        int winnerId = 0;
        if (p.size() == 2) {
            int sA = p[0].trimmed().toInt(), sB = p[1].trimmed().toInt();
            if (sA > sB)      winnerId = item->data(Qt::UserRole + 1).toInt();
            else if (sB > sA) winnerId = item->data(Qt::UserRole + 2).toInt();
        }
        auto r = logMatch_->update(item->data(Qt::UserRole).toInt(),
            "Grupos", 0, "Finalizado", winnerId, res.toStdString());
        if (r.code >= 0) { int t = cmbTournament_->currentData().toInt(); loadMatches(t); loadStandings(t); }
        else QMessageBox::warning(this, "Error", QString::fromStdString(r.message));
        });

    connect(btnEditResult_, &QPushButton::clicked, this, [this]() {
        auto* item = listMatches_->currentItem(); if (!item) return;
        bool ok;
        QString res = QInputDialog::getText(this, "Editar resultado", "Nuevo resultado:",
            QLineEdit::Normal, item->data(Qt::UserRole + 3).toString(), &ok);
        if (!ok) return;
        auto r = logMatch_->update(item->data(Qt::UserRole).toInt(),
            "Grupos", 0, "Finalizado",
            item->data(Qt::UserRole + 4).toInt(), res.toStdString());
        if (r.code >= 0) { int t = cmbTournament_->currentData().toInt(); loadMatches(t); loadStandings(t); }
        else QMessageBox::warning(this, "Error", QString::fromStdString(r.message));
        });

    connect(listMatches_, &QListWidget::itemSelectionChanged, this, [this]() {
        bool s = !listMatches_->selectedItems().isEmpty();
        btnMarkPlayed_->setEnabled(s);
        btnEditResult_->setEnabled(s);
        btnDeleteMatch_->setEnabled(s);
        });

    connect(cmbTournament_, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, [this](int) {
            int tId = cmbTournament_->currentData().toInt();
            if (tId > 0) { loadTeamsForTournament(tId); loadMatches(tId); loadStandings(tId); }
        });

    connect(btnRefreshStandings, &QPushButton::clicked, this, [this]() {
        loadStandings(cmbTournament_->currentData().toInt());
        });

    // Simulación
    auto sim = [this](auto fn) {
        int tId = cmbTournament_->currentData().toInt();
        if (tId <= 0) { QMessageBox::warning(this, "Aviso", "Selecciona un torneo primero."); return; }
        auto r = fn(tId);
        if (r.code >= 0) {
            loadMatches(tId); loadStandings(tId);
            QMessageBox::information(this, "Listo", QString::fromStdString(r.message));
        }
        else QMessageBox::warning(this, "Error", QString::fromStdString(r.message));
        };
    connect(btnGenGroups_, &QPushButton::clicked, this, [=] { sim([this](int id) { return logGame_->generateGroupMatches(id); }); });
    connect(btnPlayGroups_, &QPushButton::clicked, this, [=] { sim([this](int id) { return logGame_->playGroupMatches(id); }); });
    connect(btnGenSemi_, &QPushButton::clicked, this, [=] { sim([this](int id) { return logGame_->generateSemiMatches(id); }); });
    connect(btnPlaySemi_, &QPushButton::clicked, this, [=] { sim([this](int id) { return logGame_->playSemiMatches(id); }); });
    connect(btnGenFinal_, &QPushButton::clicked, this, [=] { sim([this](int id) { return logGame_->generateFinalMatch(id); }); });
    connect(btnPlayFinal_, &QPushButton::clicked, this, [=] { sim([this](int id) { return logGame_->playFinalMatch(id); }); });

    loadTournaments();
}

matchesDialog::~matchesDialog() {}

void matchesDialog::loadTournaments()
{
    cmbTournament_->clear();
    cmbTournament_->addItem("— Selecciona un torneo —", -1);
    for (const auto& t : logTournament_->list().data)
        cmbTournament_->addItem(QString::fromStdString(t.name), t.id);
}

void matchesDialog::loadTeamsForTournament(int tournamentId)
{
    cmbTeamA_->clear(); cmbTeamB_->clear();
    cmbTeamA_->addItem("— Equipo A —", -1);
    cmbTeamB_->addItem("— Equipo B —", -1);
    for (const auto& t : logTeam_->list().data) {
        if (t.tournamentId != tournamentId) continue;
        QString n = QString::fromStdString(t.name);
        cmbTeamA_->addItem(n, t.id);
        cmbTeamB_->addItem(n, t.id);
    }
}

void matchesDialog::loadMatches(int tournamentId)
{
    listMatches_->clear();
    auto teams = logTeam_->list();
    auto res = logMatch_->listByTournament(tournamentId);
    for (const auto& m : res.data) {
        QString tA, tB;
        for (const auto& t : teams.data) {
            if (t.id == m.teamAId) tA = QString::fromStdString(t.name);
            if (t.id == m.teamBId) tB = QString::fromStdString(t.name);
        }
        QString status = QString::fromStdString(m.status);
        QString result = QString::fromStdString(m.result.empty() ? "Pendiente" : m.result);
        QString phase = QString::fromStdString(m.phase);
        QString prefix = (status == "Finalizado") ? "[OK]" : "[--]";
        QString text = QString("%1  %2  vs  %3  —  %4  (%5, R%6)")
            .arg(prefix, tA, tB, result, phase).arg(m.round);
        QListWidgetItem* item = new QListWidgetItem(text, listMatches_);
        item->setData(Qt::UserRole, m.id);
        item->setData(Qt::UserRole + 1, m.teamAId);
        item->setData(Qt::UserRole + 2, m.teamBId);
        item->setData(Qt::UserRole + 3, result);
        item->setData(Qt::UserRole + 4, m.winnerId);
        listMatches_->addItem(item);
    }
    if (lblSubtitle_) lblSubtitle_->setText(QString("%1 partido(s)").arg(res.data.size()));
}

void matchesDialog::loadStandings(int tournamentId)
{
    if (!tableStandings_) return;
    tableStandings_->setRowCount(0);

    // Obtener equipos del torneo
    auto allTeams = logTeam_->list();

    // Filtrar por torneo y ordenar por puntos desc
    QList<Team> tourTeams;
    for (const auto& t : allTeams.data)
        if (t.tournamentId == tournamentId)
            tourTeams.append(t);

    // Ordenar por puntos descendente 
    for (int i = 0; i < tourTeams.size() - 1; i++)
        for (int j = 0; j < tourTeams.size() - 1 - i; j++)
            if (tourTeams[j].points < tourTeams[j + 1].points)
                qSwap(tourTeams[j], tourTeams[j + 1]);

    // Nombre del torneo
    auto tr = logTournament_->obtain(tournamentId);
    QString tourName = (!tr.data.empty())
        ? QString::fromStdString(tr.data.front().name) : "";

    tableStandings_->setRowCount(tourTeams.size());
    for (int row = 0; row < tourTeams.size(); row++) {
        const auto& t = tourTeams[row];
        auto setCell = [&](int col, const QString& val, Qt::AlignmentFlag align = Qt::AlignLeft) {
            QTableWidgetItem* cell = new QTableWidgetItem(val);
            cell->setTextAlignment(align | Qt::AlignVCenter);
            tableStandings_->setItem(row, col, cell);
            };
        setCell(0, QString::fromStdString(t.name));
        setCell(1, QString::number(t.points), Qt::AlignHCenter);
        setCell(2, QString::number(t.wins), Qt::AlignHCenter);
        setCell(3, QString::number(t.draws), Qt::AlignHCenter);
        setCell(4, QString::number(t.losses), Qt::AlignHCenter);
        setCell(5, tourName);
    }
}

void matchesDialog::onAddClicked()
{
    int tId = cmbTournament_->currentData().toInt();
    int tA = cmbTeamA_->currentData().toInt();
    int tB = cmbTeamB_->currentData().toInt();
    if (tId < 0 || tA < 0 || tB < 0 || tA == tB) {
        QMessageBox::warning(this, "Datos incompletos",
            "Selecciona un torneo y dos equipos distintos.");
        return;
    }
    auto r = logMatch_->insert(tId, tA, tB);
    if (r.code >= 0) loadMatches(tId);
    else QMessageBox::warning(this, "Error", QString::fromStdString(r.message));
}
void matchesDialog::onDeleteClicked()
{
    auto* item = listMatches_->currentItem(); if (!item) return;
    if (QMessageBox::warning(this, "Eliminar partido", "¿Eliminar este partido?",
        QMessageBox::Yes | QMessageBox::Cancel) == QMessageBox::Yes) {
        int tId = cmbTournament_->currentData().toInt();
        auto r = logMatch_->eliminar(item->data(Qt::UserRole).toInt());
        if (r.code >= 0) loadMatches(tId);
        else QMessageBox::warning(this, "Error", QString::fromStdString(r.message));
    }
}
void matchesDialog::onEditClicked() {}
void matchesDialog::onRefresh()
{
    loadTournaments();
    int tId = cmbTournament_->currentData().toInt();
    if (tId > 0) { loadTeamsForTournament(tId); loadMatches(tId); loadStandings(tId); }
}
