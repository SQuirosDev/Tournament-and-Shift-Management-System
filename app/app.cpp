#include "app.h"
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QGraphicsDropShadowEffect>
#include <QtWidgets/QSizePolicy>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QMessageBox>
#include <QtGui/QColor>
#include <QtGui/QFont>
#include <QtCore/QFile>
#include <QtGui/QIcon>
#include "dialogs/tournamentsdialog.h"
#include "dialogs/teamsdialog.h"
#include "dialogs/playersdialog.h"
#include "dialogs/matchesdialog.h"
#include "dialogs/dialog_manager.h"

app::app(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    this->setWindowTitle("Sistema de gestión de torneos");

    // ── Inicializar DB y capa lógica ─────────────────────────────────────────
    conn_ = new Connection();
    conn_->open("app_data.db");

    logHistoric_ = new LogHistoric(*conn_);
    logTournament_ = new LogTournament(*conn_);
    logTeam_ = new LogTeam(*conn_);
    logPlayer_ = new LogPlayer(*conn_);
    logMatch_ = new LogMatch(*conn_);
    logGame_ = new LogGame(*conn_);

    // Inyectar dependencias cruzadas
    logHistoric_->setLogTournament(logTournament_);
    logHistoric_->setLogTeam(logTeam_);
    logHistoric_->setLogPlayer(logPlayer_);
    logHistoric_->setLogMatch(logMatch_);

    logTournament_->setLogHistoric(logHistoric_);
    logTeam_->setLogHistoric(logHistoric_);
    logPlayer_->setLogHistoric(logHistoric_);
    logMatch_->setLogHistoric(logHistoric_);
    logGame_->setLogHistoric(logHistoric_);

    // ── Layout principal ─────────────────────────────────────────────────────
    QWidget* cw = ui.centralWidget;
    if (cw->layout()) delete cw->layout();

    QVBoxLayout* mainLayout = new QVBoxLayout(cw);
    mainLayout->setSpacing(28);
    mainLayout->setContentsMargins(56, 40, 56, 40);

    // ── Hero ─────────────────────────────────────────────────────────────────
    QVBoxLayout* heroLayout = new QVBoxLayout();
    heroLayout->setSpacing(10);

    QLabel* lblLogo = new QLabel(cw);
    lblLogo->setFixedSize(56, 56);
    lblLogo->setStyleSheet(
        "background-color: #0071e3; border-radius: 14px;"
        "color: white; font-size: 22pt; font-weight: bold;");
    lblLogo->setText("T");
    lblLogo->setAlignment(Qt::AlignCenter);
    heroLayout->addWidget(lblLogo, 0, Qt::AlignHCenter);

    lblWelcome = new QLabel("Sistema de gestión de torneos", cw);
    lblWelcome->setObjectName("welcomeLabel");
    lblWelcome->setWordWrap(true);
    lblWelcome->setAlignment(Qt::AlignCenter);
    heroLayout->addWidget(lblWelcome, 0, Qt::AlignHCenter);

    QLabel* lblSubTitle = new QLabel(
        "Gestione torneos, equipos, jugadores y partidos.", cw);
    lblSubTitle->setAlignment(Qt::AlignCenter);
    lblSubTitle->setStyleSheet("color: #86868b; font-size: 10pt;");
    heroLayout->addWidget(lblSubTitle, 0, Qt::AlignHCenter);

    mainLayout->addLayout(heroLayout);

    // ── Cards grid ───────────────────────────────────────────────────────────
    // Botones guardados en ui struct
    ui.btnTournaments = new QPushButton("Gestionar", cw);
    ui.btnTeams = new QPushButton("Gestionar", cw);
    ui.btnPlayers = new QPushButton("Gestionar", cw);
    ui.btnMatches = new QPushButton("Gestionar", cw);

    ui.btnTournaments->setObjectName("btnTournaments");
    ui.btnTeams->setObjectName("btnTeams");
    ui.btnPlayers->setObjectName("btnPlayers");
    ui.btnMatches->setObjectName("btnMatches");

    struct CardDef { QString title; QString subtitle; QPushButton* btn; bool primary; };
    QList<CardDef> cards = {
        { "Torneos",   "Crea y administra torneos",       ui.btnTournaments, true  },
        { "Equipos",   "Asigna equipos a torneos",        ui.btnTeams,       false },
        { "Jugadores", "Registra jugadores por equipo",   ui.btnPlayers,     false },
        { "Partidos",  "Programa y gestiona encuentros",  ui.btnMatches,     false },
    };

    QGridLayout* grid = new QGridLayout();
    grid->setSpacing(20);

    int col = 0, row = 0;
    for (auto& c : cards) {
        QWidget* card = new QWidget(cw);
        card->setStyleSheet(
            "QWidget { background: #ffffff; border: 1px solid #e6e6ea;"
            "  border-radius: 16px; }");

        QVBoxLayout* cl = new QVBoxLayout(card);
        cl->setContentsMargins(20, 18, 20, 18);
        cl->setSpacing(6);

        QLabel* lTitle = new QLabel(c.title, card);
        lTitle->setStyleSheet("font-size: 13pt; font-weight: 700; color: #1d1d1f; border: none;");

        QLabel* lSub = new QLabel(c.subtitle, card);
        lSub->setStyleSheet("font-size: 9pt; color: #86868b; border: none;");
        lSub->setWordWrap(true);

        c.btn->setParent(card);
        c.btn->setCursor(Qt::PointingHandCursor);
        c.btn->setMinimumHeight(34);

        if (c.primary) {
            c.btn->setStyleSheet(
                "QPushButton { background:#0071e3; color:white; border:none;"
                "  border-radius:8px; padding:6px 16px; font-size:10pt; font-weight:600; }"
                "QPushButton:hover { background:#0077ed; }"
                "QPushButton:disabled { background:#b3d4f5; color:white; }");
        }
        else {
            c.btn->setStyleSheet(
                "QPushButton { background:#f5f5f7; color:#1d1d1f; border:1px solid #e6e6ea;"
                "  border-radius:8px; padding:6px 16px; font-size:10pt; font-weight:600; }"
                "QPushButton:hover { background:#e8e8ed; }"
                "QPushButton:disabled { color:#b0b0b8; background:#f5f5f7; }");
        }

        cl->addWidget(lTitle);
        cl->addWidget(lSub);
        cl->addSpacing(8);
        cl->addWidget(c.btn, 0, Qt::AlignLeft);

        grid->addWidget(card, row, col);
        col++;
        if (col == 2) { col = 0; row++; }
    }
    mainLayout->addLayout(grid);
    mainLayout->addStretch();

    // ── Estilos globales ─────────────────────────────────────────────────────
    qApp->setFont(QFont("Segoe UI Variable", 10));
    qApp->setStyleSheet(R"css(
        QMainWindow { background-color: #f5f5f7; }

        QWidget#centralWidget {
            background-color: #ffffff;
            border: 1px solid #d2d2d7;
            border-radius: 20px;
        }

        QLabel#welcomeLabel {
            color: #1d1d1f;
            font-size: 15pt;
            font-weight: 700;
        }

        /* ── Fix texto invisible en listas ── */
        QListWidget {
            border: 1px solid #e6e6ea;
            border-radius: 12px;
            background-color: #ffffff;
            padding: 6px;
            outline: none;
            color: #1d1d1f;
        }
        QListWidget::item {
            border-radius: 8px;
            padding: 10px 12px;
            margin: 1px 0;
            color: #1d1d1f;
        }
        QListWidget::item:selected {
            background-color: #e8f2ff;
            color: #0071e3;
        }
        QListWidget::item:hover:!selected {
            background-color: #f5f5f7;
            color: #1d1d1f;
        }

        QLineEdit, QComboBox {
            background-color: #fbfbfd;
            border: 1px solid #e6e6ea;
            border-radius: 10px;
            padding: 8px 12px;
            color: #1d1d1f;
            font-size: 10pt;
        }
        QLineEdit:focus, QComboBox:focus {
            border: 2px solid #0071e3;
            background-color: #ffffff;
        }

        QDialog { background-color: #f5f5f7; }

        QMessageBox { background-color: #ffffff; }
        QMessageBox QLabel { color: #1d1d1f; font-size: 10pt; }
        QMessageBox QPushButton {
            background-color: #f5f5f7;
            border: 1px solid #e6e6ea;
            border-radius: 8px;
            padding: 6px 14px;
            color: #1d1d1f;
        }

        QScrollBar:vertical {
            border: none; background: #f5f5f7;
            width: 8px; margin: 0; border-radius: 4px;
        }
        QScrollBar::handle:vertical {
            background: #d2d2d7; min-height: 20px; border-radius: 4px;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
    )css");

    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(32);
    shadow->setOffset(0, 8);
    shadow->setColor(QColor(0, 0, 0, 16));
    ui.centralWidget->setGraphicsEffect(shadow);

    // ── Conexiones ───────────────────────────────────────────────────────────
    connect(ui.btnTournaments, &QPushButton::clicked, this, [this]() {
        auto before = logTournament_->list();
        bool hadBefore = !before.data.empty();

        tournamentsDialog dlg(logTournament_, this);
        DialogManager::openModal(&dlg);
        updateUiState();

        // Si no había torneos y ahora hay, abrir equipos automáticamente
        auto after = logTournament_->list();
        if (!hadBefore && !after.data.empty()) {
            teamsDialog dlg2(logTeam_, logTournament_, this);
            DialogManager::openModal(&dlg2);
            updateUiState();
        }
        });

    connect(ui.btnTeams, &QPushButton::clicked, this, [this]() {
        teamsDialog dlg(logTeam_, logTournament_, this);
        DialogManager::openModal(&dlg);
        updateUiState();
        });

    connect(ui.btnPlayers, &QPushButton::clicked, this, [this]() {
        playersDialog dlg(logPlayer_, logTeam_, this);
        DialogManager::openModal(&dlg);
        updateUiState();
        });

    connect(ui.btnMatches, &QPushButton::clicked, this, [this]() {
        matchesDialog dlg(logMatch_, logGame_, logTournament_, logTeam_, this);
        DialogManager::openModal(&dlg);
        updateUiState();
        });

    updateUiState();
}

app::~app()
{
    delete logGame_;
    delete logMatch_;
    delete logPlayer_;
    delete logTeam_;
    delete logTournament_;
    delete logHistoric_;
    delete conn_;
}

void app::updateUiState()
{
    if (!conn_ || !lblWelcome) return;

    auto tourRes = logTournament_->list();
    bool hasTournaments = !tourRes.data.empty();
    bool hasTeams = false;
    bool hasPlayers = false;

    if (hasTournaments) {
        int tId = tourRes.data.front().id;
        auto teamsRes = conn_->listTeamsByTournament(tId);
        hasTeams = (teamsRes.code >= 0 && !teamsRes.data.empty());
        if (hasTeams) {
            int teamId = teamsRes.data.front().id;
            auto plRes = conn_->listPlayersByTeam(teamId);
            hasPlayers = (plRes.code >= 0 && !plRes.data.empty());
        }
    }

    ui.btnTeams->setVisible(hasTournaments);
    ui.btnTeams->setEnabled(hasTournaments);
    ui.btnPlayers->setVisible(hasTournaments && hasTeams);
    ui.btnPlayers->setEnabled(hasTournaments && hasTeams);
    ui.btnMatches->setVisible(hasTournaments && hasTeams && hasPlayers);
    ui.btnMatches->setEnabled(hasTournaments && hasTeams && hasPlayers);

    if (!hasTournaments) {
        lblWelcome->setText("Bienvenido — crea tu primer torneo para comenzar.");
        lblWelcome->setStyleSheet("color: #1d1d1f; font-weight: 700; font-size: 15pt;");
    }
    else if (!hasTeams) {
        lblWelcome->setText("Torneo creado. Agrega equipos para continuar.");
        lblWelcome->setStyleSheet("color: #0071e3; font-weight: 700; font-size: 15pt;");
    }
    else if (!hasPlayers) {
        lblWelcome->setText("Equipos listos. Registra jugadores para generar partidos.");
        lblWelcome->setStyleSheet("color: #0071e3; font-weight: 700; font-size: 15pt;");
    }
    else {
        lblWelcome->setText("Panel de control — torneo activo.");
        lblWelcome->setStyleSheet("color: #0071e3; font-weight: 700; font-size: 15pt;");
    }
}

// Slots legacy (ya no se usan, se mantienen para no romper el .h)
void app::onPlayersClicked() {}
void app::onTeamsClicked() {}
void app::onTournamentsClicked() {}
void app::onMatchesClicked() {}
