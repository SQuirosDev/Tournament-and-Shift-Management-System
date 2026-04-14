#include "app.h"
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QGraphicsDropShadowEffect>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QMessageBox>
#include <QtGui/QColor>
#include <QtGui/QFont>
#include "dialogs/tournamentsdialog.h"
#include "dialogs/teamsdialog.h"
#include "dialogs/playersdialog.h"
#include "dialogs/matchesdialog.h"
#include "dialogs/dialog_manager.h"

app::app(QWidget* parent) : QMainWindow(parent)
{
    ui.setupUi(this);
    this->setWindowTitle("Sistema de gestión de torneos");

    // ── Capa lógica ──────────────────────────────────────────────────────────
    
    conn_ = new Connection();

    DbResponse dbResponse = conn_->open("app_data.db");

    if (dbResponse.code != CODE_SUCCESS) {
        QMessageBox::critical(this, "Error", "No se pudo abrir la base de datos");
    }

    logHistoric_ = new LogHistoric(*conn_);
    logTournament_ = new LogTournament(*conn_);
    logTeam_ = new LogTeam(*conn_);
    logPlayer_ = new LogPlayer(*conn_);
    logMatch_ = new LogMatch(*conn_);
    logGame_ = new LogGame(*conn_);

    logTournament_->setLogHistoric(logHistoric_);
    logTeam_->setLogHistoric(logHistoric_);
    logPlayer_->setLogHistoric(logHistoric_);
    logMatch_->setLogHistoric(logHistoric_);
    logGame_->setLogHistoric(logHistoric_);

    // ── Layout ───────────────────────────────────────────────────────────────
    QWidget* cw = ui.centralWidget;
    if (cw->layout()) delete cw->layout();
    QVBoxLayout* root = new QVBoxLayout(cw);
    root->setSpacing(22);
    root->setContentsMargins(48, 36, 48, 36);

    // Hero
    QVBoxLayout* hero = new QVBoxLayout();
    hero->setSpacing(8);
    QLabel* lblLogo = new QLabel("T", cw);
    lblLogo->setFixedSize(52, 52);
    lblLogo->setAlignment(Qt::AlignCenter);
    lblLogo->setStyleSheet(
        "background:#0071e3; border-radius:13px;"
        "color:white; font-size:20pt; font-weight:bold;");
    hero->addWidget(lblLogo, 0, Qt::AlignHCenter);

    lblWelcome = new QLabel("", cw);
    lblWelcome->setObjectName("welcomeLabel");
    lblWelcome->setAlignment(Qt::AlignCenter);
    lblWelcome->setWordWrap(true);
    lblWelcome->setMinimumHeight(28);
    hero->addWidget(lblWelcome, 0, Qt::AlignHCenter);

    QLabel* lblSub = new QLabel("Gestione torneos, equipos, jugadores y partidos.", cw);
    lblSub->setAlignment(Qt::AlignCenter);
    lblSub->setStyleSheet("color:#86868b; font-size:10pt;");
    hero->addWidget(lblSub, 0, Qt::AlignHCenter);
    root->addLayout(hero);

    // Cards
    ui.btnTournaments = new QPushButton("Gestionar", cw);
    ui.btnTeams = new QPushButton("Gestionar", cw);
    ui.btnPlayers = new QPushButton("Gestionar", cw);
    ui.btnMatches = new QPushButton("Gestionar", cw);
    ui.btnTournaments->setObjectName("btnTournaments");
    ui.btnTeams->setObjectName("btnTeams");
    ui.btnPlayers->setObjectName("btnPlayers");
    ui.btnMatches->setObjectName("btnMatches");

    struct CardDef { QString title, sub; QPushButton* btn; bool primary; };
    QList<CardDef> cards = {
        {"Torneos",   "Crea y administra torneos",      ui.btnTournaments, true },
        {"Equipos",   "Asigna equipos a torneos",       ui.btnTeams,       false},
        {"Jugadores", "Registra jugadores por equipo",  ui.btnPlayers,     false},
        {"Partidos",  "Programa y gestiona encuentros", ui.btnMatches,     false},
    };
    QGridLayout* grid = new QGridLayout();
    grid->setSpacing(14);
    int col = 0, row = 0;
    for (auto& c : cards) {
        QWidget* card = new QWidget(cw);
        card->setStyleSheet(
            "QWidget{background:#ffffff;border:1px solid #e0e0e8;border-radius:14px;}");
        QVBoxLayout* cl = new QVBoxLayout(card);
        cl->setContentsMargins(18, 15, 18, 15);
        cl->setSpacing(4);
        QLabel* lt = new QLabel(c.title, card);
        lt->setStyleSheet("font-size:12pt;font-weight:700;color:#1d1d1f;border:none;");
        QLabel* ls = new QLabel(c.sub, card);
        ls->setStyleSheet("font-size:9pt;color:#86868b;border:none;");
        ls->setWordWrap(true);
        c.btn->setParent(card);
        c.btn->setCursor(Qt::PointingHandCursor);
        c.btn->setMinimumHeight(32);
        c.btn->setMaximumWidth(115);
        c.btn->setStyleSheet(c.primary
            ? "QPushButton{background:#0071e3;color:#fff;border:none;border-radius:8px;"
            "padding:6px 14px;font-size:10pt;font-weight:600;}"
            "QPushButton:hover{background:#0077ed;}"
            "QPushButton:disabled{background:#b3d4f5;}"
            : "QPushButton{background:#f5f5f7;color:#1d1d1f;border:1px solid #d2d2d7;"
            "border-radius:8px;padding:6px 14px;font-size:10pt;font-weight:600;}"
            "QPushButton:hover{background:#e8e8ed;}"
            "QPushButton:disabled{color:#b0b0b8;background:#f5f5f7;}");
        cl->addWidget(lt); cl->addWidget(ls); cl->addSpacing(6);
        cl->addWidget(c.btn, 0, Qt::AlignLeft);
        grid->addWidget(card, row, col);
        if (++col == 2) { col = 0; row++; }
    }
    root->addLayout(grid);
    root->addStretch();

    // ── Estilos globales ─────────────────────────────────────────────────────
    qApp->setFont(QFont("Segoe UI Variable", 10));
    qApp->setStyleSheet(R"css(
        QMainWindow { background-color: #f0f0f5; }
        QWidget#centralWidget {
            background-color: #ffffff;
            border: 1px solid #d2d2d7;
            border-radius: 18px;
        }
        QLabel#welcomeLabel {
            font-size: 14pt; font-weight: 700; color: #1d1d1f;
        }
        /* Listas */
        QListWidget {
            border:1px solid #d2d2d7; border-radius:12px;
            background:#ffffff; padding:4px; outline:none; color:#1d1d1f;
        }
        QListWidget::item {
            border-radius:8px; padding:10px 12px; margin:1px 0; color:#1d1d1f;
        }
        QListWidget::item:selected { background:#e8f2ff; color:#0071e3; }
        QListWidget::item:hover:!selected { background:#f0f0f5; color:#1d1d1f; }
        /* ComboBox + popup visible */
        QComboBox {
            background:#ffffff; border:1px solid #d2d2d7;
            border-radius:10px; padding:8px 12px;
            font-size:10pt; color:#1d1d1f;
        }
        QComboBox:focus { border:2px solid #0071e3; }
        QComboBox::drop-down { border:none; width:20px; }
        QComboBox QAbstractItemView {
            background:#ffffff; color:#1d1d1f;
            border:1px solid #d2d2d7; border-radius:8px;
            selection-background-color:#e8f2ff;
            selection-color:#0071e3; outline:none;
        }
        /* Inputs */
        QLineEdit {
            background:#ffffff; border:1px solid #d2d2d7;
            border-radius:10px; padding:8px 12px;
            color:#1d1d1f; font-size:10pt;
        }
        QLineEdit:focus { border:2px solid #0071e3; }
        /* Tablas */
        QTableWidget {
            border:1px solid #d2d2d7; border-radius:12px;
            background:#ffffff; gridline-color:#f0f0f5;
            color:#1d1d1f; outline:none;
        }
        QTableWidget::item { padding:8px 10px; color:#1d1d1f; }
        QTableWidget::item:selected { background:#e8f2ff; color:#0071e3; }
        QHeaderView::section {
            background:#f5f5f7; color:#1d1d1f; font-weight:600;
            border:none; border-bottom:1px solid #d2d2d7; padding:8px 10px;
        }
        QDialog { background:#f5f5f7; }
        QMessageBox { background:#ffffff; }
        QMessageBox QLabel { color:#1d1d1f; font-size:10pt; }
        QMessageBox QPushButton {
            background:#f5f5f7; border:1px solid #d2d2d7;
            border-radius:8px; padding:6px 14px; color:#1d1d1f;
        }
        QScrollBar:vertical {
            border:none; background:#f5f5f7; width:8px; border-radius:4px;
        }
        QScrollBar::handle:vertical {
            background:#d2d2d7; min-height:20px; border-radius:4px;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height:0; }
        QTabWidget::pane { border:1px solid #d2d2d7; border-radius:0 0 12px 12px; background:#ffffff; }
        QTabBar::tab {
            background:#f5f5f7; color:#86868b; border:1px solid #d2d2d7;
            border-bottom:none; border-radius:8px 8px 0 0;
            padding:7px 16px; font-size:10pt; font-weight:600;
        }
        QTabBar::tab:selected { background:#ffffff; color:#0071e3; }
        QTabBar::tab:hover:!selected { background:#e8e8ed; color:#1d1d1f; }
    )css");

    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(28); shadow->setOffset(0, 6);
    shadow->setColor(QColor(0, 0, 0, 14));
    ui.centralWidget->setGraphicsEffect(shadow);

    // ── Conexiones ───────────────────────────────────────────────────────────
    connect(ui.btnTournaments, &QPushButton::clicked, this, [this]() {
        bool hadBefore = !logTournament_->list().data.empty();
        tournamentsDialog dlg(logTournament_, this);
        DialogManager::openModal(&dlg);
        updateUiState();
        if (!hadBefore && !logTournament_->list().data.empty()) {
            teamsDialog dlg2(logTeam_, logTournament_, this);
            DialogManager::openModal(&dlg2);
            updateUiState();
        }
        });
    connect(ui.btnTeams, &QPushButton::clicked, this, [this]() {
        teamsDialog dlg(logTeam_, logTournament_, this);
        DialogManager::openModal(&dlg); updateUiState();
        });
    connect(ui.btnPlayers, &QPushButton::clicked, this, [this]() {
        playersDialog dlg(logPlayer_, logTeam_, this);
        DialogManager::openModal(&dlg); updateUiState();
        });
    connect(ui.btnMatches, &QPushButton::clicked, this, [this]() {
        matchesDialog dlg(logMatch_, logGame_, logTournament_, logTeam_, this);
        DialogManager::openModal(&dlg); updateUiState();
        });

    updateUiState();
}

app::~app()
{
    delete logGame_; delete logMatch_; delete logPlayer_;
    delete logTeam_; delete logTournament_; delete logHistoric_; delete conn_;
}

void app::updateUiState()
{
    if (!conn_ || !lblWelcome) return;
    auto tourRes = logTournament_->list();
    bool hasTournaments = !tourRes.data.empty();
    bool hasTeams = false, hasPlayers = false;
    if (hasTournaments) {
        auto tr = conn_->listTeamsByTournament(tourRes.data.front().id);
        hasTeams = (tr.code >= 0 && !tr.data.empty());
        if (hasTeams) {
            auto pr = conn_->listPlayersByTeam(tr.data.front().id);
            hasPlayers = (pr.code >= 0 && !pr.data.empty());
        }
    }
    ui.btnTeams->setVisible(hasTournaments);
    ui.btnTeams->setEnabled(hasTournaments);
    ui.btnPlayers->setVisible(hasTournaments && hasTeams);
    ui.btnPlayers->setEnabled(hasTournaments && hasTeams);
    ui.btnMatches->setVisible(hasTournaments && hasTeams && hasPlayers);
    ui.btnMatches->setEnabled(hasTournaments && hasTeams && hasPlayers);

    if (!hasTournaments)
        lblWelcome->setText("Bienvenido — crea tu primer torneo.");
    else if (!hasTeams)
        lblWelcome->setText("Torneo creado. Agrega equipos para continuar.");
    else if (!hasPlayers)
        lblWelcome->setText("Equipos listos. Registra jugadores.");
    else
        lblWelcome->setText("Panel de control");
}

void app::onPlayersClicked() {}
void app::onTeamsClicked() {}
void app::onTournamentsClicked() {}
void app::onMatchesClicked() {}
