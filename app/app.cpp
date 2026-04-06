#include "app.h"
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QGraphicsDropShadowEffect>
#include <QtWidgets/QSizePolicy>
#include <QtWidgets/QVBoxLayout>
#include <QtGui/QColor>
#include <QtGui/QFont>
#include <QtCore/QFile>
#include <QtGui/QIcon>
#include "dialogs/tournamentsdialog.h"
#include "dialogs/teamsdialog.h"
#include "dialogs/playersdialog.h"
#include "dialogs/matchesdialog.h"
#include "dialogs/dialog_manager.h"
#include "connection.h"

app::app(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    // Window title and icon (Spanish)
    this->setWindowTitle("Sistema de gestión de torneos");
    // try to set a window icon from resources or file
    QIcon winIcon;
    QStringList iconCandidates = {
        QStringLiteral(":/app/resources/icons/trophy.svg"),
        QStringLiteral("resources/icons/trophy.svg"),
        QStringLiteral("app/resources/icons/trophy.svg")
    };
    for (const QString &p : iconCandidates) {
        if (QFile::exists(p)) { winIcon = QIcon(p); break; }
    }
    if (!winIcon.isNull()) this->setWindowIcon(winIcon);
    // Build runtime-only layout and widgets (ignore Designer state). This ensures
    // consistent layout regardless of modifications in Designer.
    QWidget* cw = ui.centralWidget;
    QVBoxLayout* mainLayout = nullptr;
    if (cw) {
        // Replace any existing layout with our runtime layout
        if (cw->layout()) {
            delete cw->layout();
        }
        mainLayout = new QVBoxLayout(cw);
        mainLayout->setSpacing(32);
        mainLayout->setContentsMargins(64, 48, 64, 48);
        cw->setLayout(mainLayout);
    }

    // Hero Section: Logo Placeholder + Welcome Title
    QVBoxLayout* heroLayout = new QVBoxLayout();
    heroLayout->setSpacing(12);

    QLabel* lblLogo = new QLabel(cw);
    lblLogo->setFixedSize(64, 64);
    lblLogo->setStyleSheet("background-color: #0071e3; border-radius: 16px; color: white; font-size: 24pt; font-weight: bold;");
    lblLogo->setText("T");
    lblLogo->setAlignment(Qt::AlignCenter);
    heroLayout->addWidget(lblLogo, 0, Qt::AlignHCenter);

    // Ensure welcome label exists and is first in the layout
    QLabel* runtimeWelcome = nullptr;
    if (cw) runtimeWelcome = cw->findChild<QLabel*>("welcomeLabel");
    if (!runtimeWelcome) {
        runtimeWelcome = new QLabel(tr("Tournament Manager Pro"), cw);
        runtimeWelcome->setObjectName("welcomeLabel");
        runtimeWelcome->setWordWrap(true);
        runtimeWelcome->setAlignment(Qt::AlignCenter);
    }
    heroLayout->addWidget(runtimeWelcome, 0, Qt::AlignHCenter);
    
    QLabel* lblSubTitle = new QLabel(tr("Configure su torneo y gestione equipos, jugadores y partidos de forma intuitiva."), cw);
    lblSubTitle->setAlignment(Qt::AlignCenter);
    lblSubTitle->setStyleSheet("color: #86868b; font-size: 11pt; font-weight: 400;");
    heroLayout->addWidget(lblSubTitle, 0, Qt::AlignHCenter);

    if (mainLayout) {
        mainLayout->addLayout(heroLayout);
        mainLayout->addSpacing(24);
    }

    // Helper to get or create a button
    auto getButton = [&](const char* name, const QString& text, bool isPrimary = false)->QPushButton* {
        QPushButton* b = nullptr;
        // Prefer existing pointer from ui struct
        if (strcmp(name, "btnTournaments") == 0 && ui.btnTournaments) b = ui.btnTournaments;
        if (strcmp(name, "btnTeams") == 0 && ui.btnTeams) b = ui.btnTeams;
        if (strcmp(name, "btnPlayers") == 0 && ui.btnPlayers) b = ui.btnPlayers;
        if (strcmp(name, "btnMatches") == 0 && ui.btnMatches) b = ui.btnMatches;
        if (!b && cw) b = cw->findChild<QPushButton*>(name);
        if (!b) {
            b = new QPushButton(text, cw);
            b->setObjectName(name);
        } else {
            b->setText(text);
        }
        b->setMinimumHeight(48);
        b->setMinimumWidth(220);
        b->setCursor(Qt::PointingHandCursor);
        if (isPrimary) {
            b->setProperty("primary", true);
        }
        b->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        return b;
    };

    QPushButton* btnT = getButton("btnTournaments", tr("Gestionar Torneos"), true);
    QPushButton* btnTe = getButton("btnTeams", tr("Gestionar Equipos"));
    QPushButton* btnP = getButton("btnPlayers", tr("Gestionar Jugadores"));
    QPushButton* btnM = getButton("btnMatches", tr("Gestionar Partidos"));

    // Store back into ui struct so other code can use ui.btnX
    ui.btnTournaments = btnT;
    ui.btnTeams = btnTe;
    ui.btnPlayers = btnP;
    ui.btnMatches = btnM;

    // Buttons grid layout for better organization
    QGridLayout* buttonsGrid = new QGridLayout();
    buttonsGrid->setSpacing(24);

    // Button + description pairs to improve visual flow (Card style)
    auto addActionCard = [&](QPushButton* b, const QString& title, const QString& iconText, int row, int col) {
        // Card layout: Title, Icon, Description, Spacer, Centered Button
        QVBoxLayout* cardLayout = new QVBoxLayout();
        cardLayout->setSpacing(10);
        cardLayout->setContentsMargins(12,12,12,12);

        QLabel* lblTitle = new QLabel(title, cw);
        lblTitle->setStyleSheet("color: #1d1d1f; font-size: 11pt; font-weight: 700;");
        lblTitle->setAlignment(Qt::AlignLeft);
        lblTitle->setContentsMargins(6,6,6,6);
        cardLayout->addWidget(lblTitle);

        // Icon placeholder: use resource icons if available, fallback to circular badge
        QLabel* icon = new QLabel(cw);
        icon->setFixedSize(56,56);
        icon->setAlignment(Qt::AlignCenter);
        QPixmap pix;
        // try load resource path (:/app/resources/icons/<name>.svg)
        QString iconPath;
        if (iconText == QString::fromUtf8("🏆")) iconPath = QStringLiteral(":/app/resources/icons/trophy.svg");
        else if (iconText == QString::fromUtf8("👥")) iconPath = QStringLiteral(":/app/resources/icons/teams.svg");
        else if (iconText == QString::fromUtf8("👤")) iconPath = QStringLiteral(":/app/resources/icons/player.svg");
        else if (iconText == QString::fromUtf8("⚽")) iconPath = QStringLiteral(":/app/resources/icons/matches.svg");

        if (!iconPath.isEmpty()) {
            pix.load(iconPath);
        }
        if (!pix.isNull()) {
            icon->setPixmap(pix.scaled(48,48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            icon->setText(iconText);
            icon->setStyleSheet("background-color: #0071e3; color: white; border-radius: 28px; font-weight:700; font-size:20pt;");
        }
        cardLayout->addWidget(icon, 0, Qt::AlignHCenter);

        // (No verbose description - keep UI minimal: title, icon, action button)

        cardLayout->addStretch();

        // Center the action button and give it a uniform look
        b->setMinimumWidth(140);
        b->setMinimumHeight(36);
        b->setCursor(Qt::PointingHandCursor);

        // Override button visuals to avoid nested rounded placeholders
        bool isPrimary = b->property("primary").toBool();
        if (isPrimary) {
            b->setStyleSheet("background-color: #0071e3; color: white; border: none; border-radius: 8px; padding: 6px 14px;");
        } else {
            b->setStyleSheet("background-color: #ffffff; color: #1d1d1f; border: 1px solid #e6e6ea; border-radius: 8px; padding: 6px 14px;");
        }

        QWidget* btnWrapper = new QWidget(cw);
        btnWrapper->setStyleSheet("background: transparent;");
        QHBoxLayout* btnLay = new QHBoxLayout(btnWrapper);
        btnLay->setContentsMargins(0,0,0,0);
        btnLay->addStretch();
        btnLay->addWidget(b);
        btnLay->addStretch();
        cardLayout->addWidget(btnWrapper);

        QWidget* cardWidget = new QWidget(cw);
        cardWidget->setLayout(cardLayout);
        cardWidget->setStyleSheet("background-color: #fbfbfd; border: 1px solid #e6e6ea; border-radius: 16px; padding: 16px;");

        buttonsGrid->addWidget(cardWidget, row, col);
    };

    // Use emoji/symbols for icons: 🏆 Torneos, 👥 Equipos, 👤 Jugadores, ⚽ Partidos
    addActionCard(btnT, tr("Torneos"), QString::fromUtf8("🏆"), 0, 0);
    addActionCard(btnTe, tr("Equipos"), QString::fromUtf8("👥"), 0, 1);
    addActionCard(btnP, tr("Jugadores"), QString::fromUtf8("👤"), 1, 0);
    addActionCard(btnM, tr("Partidos"), QString::fromUtf8("⚽"), 1, 1);

    if (mainLayout) {
        mainLayout->addLayout(buttonsGrid);
        mainLayout->addStretch();
    }
    // Initialize DB connection
    conn_ = new Connection();
    conn_->open("app_data.db");

    // Global styling inspired by Apple-style aesthetics and modern UI trends
    qApp->setFont(QFont("Segoe UI Variable", 10));
    qApp->setStyleSheet(R"css(
        /* Global backdrop */
        QMainWindow { 
            background-color: #f5f5f7;
        }

        /* Central modern card container */
        QWidget#centralWidget {
            background-color: #ffffff;
            border: 1px solid #d2d2d7;
            border-radius: 24px;
        }

        /* Welcome title / Hero text */
        QLabel#welcomeLabel {
            color: #1d1d1f;
            font-size: 16pt;
            font-weight: 700;
            margin-bottom: 8px;
        }

        /* Input aesthetics */
        QLineEdit, QComboBox, QSpinBox {
            background-color: #fbfbfd;
            border: 1px solid #d2d2d7;
            border-radius: 12px;
            padding: 10px 14px;
            color: #1d1d1f;
            font-size: 10pt;
        }
        QLineEdit:focus, QComboBox:focus {
            border: 2px solid #0071e3;
            background-color: #ffffff;
        }

        /* Standard Buttons - Modern Glassmorphism-lite */
        QPushButton {
            background-color: #f5f5f7;
            border: 1px solid transparent;
            border-radius: 12px;
            color: #1d1d1f;
            font-weight: 600;
            font-size: 10pt;
            padding: 10px 24px;
        }
        QPushButton:hover {
            background-color: #e8e8ed;
        }
        QPushButton:pressed {
            background-color: #d2d2d7;
        }
        QPushButton:disabled {
            background-color: #f5f5f7;
            color: #d2d2d7;
            border: 1px solid #e8e8ed;
        }

        /* Primary Call-to-Action (Modern Blue) */
        QPushButton[primary="true"] {
            background-color: #0071e3;
            color: #ffffff;
            border: none;
        }
        QPushButton[primary="true"]:hover {
            background-color: #0077ed;
        }
        QPushButton[primary="true"]:pressed {
            background-color: #0062c3;
        }

        /* Danger action style */
        QPushButton[danger="true"] {
            background-color: #ff3b30;
            color: #ffffff;
        }
        QPushButton[danger="true"]:hover {
            background-color: #ff453a;
        }

        /* Dialogs and Popups */
        QDialog {
            background-color: #ffffff;
            border-radius: 20px;
        }

        /* Message boxes */
        QMessageBox {
            background-color: #ffffff;
            color: #1d1d1f;
        }
        QMessageBox QLabel#qt_msgbox_label, QMessageBox QLabel#qt_msgboxexplainlabel, QMessageBox QLabel {
            color: #1d1d1f;
            font-size: 10pt;
        }
        QMessageBox QPushButton {
            background-color: #f5f5f7;
            border: 1px solid #e6e6ea;
            border-radius: 8px;
            padding: 6px 12px;
        }

        /* List styling */
        QListWidget {
            border: 1px solid #d2d2d7;
            border-radius: 14px;
            background-color: #ffffff;
            padding: 8px;
            outline: none;
        }
        QListWidget::item {
            border-radius: 8px;
            padding: 10px;
            margin: 2px 0px;
        }
        QListWidget::item:selected {
            background-color: #e8f2ff;
            color: #0071e3;
        }

        /* Custom ScrollBar for modern look */
        QScrollBar:vertical {
            border: none;
            background: #f5f5f7;
            width: 8px;
            margin: 0px;
            border-radius: 4px;
        }
        QScrollBar::handle:vertical {
            background: #d2d2d7;
            min-height: 20px;
            border-radius: 4px;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }
    )css");

    // Apply a cleaner, more subtle shadow effect
    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(40);
    shadow->setOffset(0, 12);
    shadow->setColor(QColor(0, 0, 0, 18));
    ui.centralWidget->setGraphicsEffect(shadow);

    lblWelcome = runtimeWelcome; // Link to the label we created above

    // Update UI state based on whether tournaments exist
    updateUiState();

    // Ensure buttons connect after being (re)created
    connect(ui.btnPlayers, &QPushButton::clicked, this, [this]() {
        playersDialog dlg(conn_, this);
        DialogManager::openModal(&dlg);
        updateUiState();
    });
    connect(ui.btnTeams, &QPushButton::clicked, this, [this]() {
        teamsDialog dlg(conn_, this);
        DialogManager::openModal(&dlg);
        updateUiState();
    });
    connect(ui.btnTournaments, &QPushButton::clicked, this, [this]() {
        // detect if we had any tournaments before opening the dialog
        auto before = conn_->listTournaments();
        bool hadBefore = (before.code >= 0 && !before.data.empty());

        tournamentsDialog dlg(conn_, this);
        DialogManager::openModal(&dlg);

        // after closing, refresh UI
        updateUiState();

        // if there were no tournaments before and now there are, continue the flow to Teams
        auto after = conn_->listTournaments();
        bool hasNow = (after.code >= 0 && !after.data.empty());
        if (!hadBefore && hasNow) {
            teamsDialog dlg2(conn_, this);
            DialogManager::openModal(&dlg2);
            updateUiState();
        }
    });
    connect(ui.btnMatches, &QPushButton::clicked, this, [this]() {
        matchesDialog dlg(conn_, this);
        DialogManager::openModal(&dlg);
        updateUiState();
    });

    // Mark the primary action visually
    ui.btnTournaments->setProperty("primary", true);
    ui.btnTournaments->style()->unpolish(ui.btnTournaments);
    ui.btnTournaments->style()->polish(ui.btnTournaments);

    // Initial visibility / enabled state
    updateUiState();
}

app::~app()
{}

void app::updateUiState()
{
    if (!conn_ || !lblWelcome) return;
    // Check whether there are any tournaments in DB to enable other operations
    auto res = conn_->listTournaments();
    bool hasTournaments = (res.code >= 0 && !res.data.empty());

    // By default disable all secondary actions
    bool hasTeams = false;
    bool hasPlayers = false;

    if (hasTournaments) {
        // Use first tournament as active context for enabling other features
        int tournamentId = res.data.front().id;
        auto teamsRes = conn_->listTeamsByTournament(tournamentId);
        hasTeams = (teamsRes.code >= 0 && !teamsRes.data.empty());
        if (hasTeams) {
            // Use first team as context to check for players
            int teamId = teamsRes.data.front().id;
            auto playersRes = conn_->listPlayersByTeam(teamId);
            hasPlayers = (playersRes.code >= 0 && !playersRes.data.empty());
        }
    }

    ui.btnTeams->setVisible(hasTournaments);
    ui.btnTeams->setEnabled(hasTournaments);
    ui.btnPlayers->setVisible(hasTournaments && hasTeams);
    ui.btnPlayers->setEnabled(hasTournaments && hasTeams && hasPlayers);
    ui.btnMatches->setVisible(hasTournaments && hasTeams && hasPlayers);

    if (!hasTournaments) {
        lblWelcome->setText("¡Bienvenido! Empiece creando su primer torneo.");
        lblWelcome->setStyleSheet("color: #1d1d1f; font-weight: 700;");
    } else if (!hasTeams) {
        lblWelcome->setText("Torneo creado. Ahora cree equipos para comenzar a registrar jugadores.");
        lblWelcome->setStyleSheet("color: #0071e3; font-weight: 700;");
    } else if (!hasPlayers) {
        lblWelcome->setText("Equipos creados. Registre jugadores para empezar a generar partidos.");
        lblWelcome->setStyleSheet("color: #0071e3; font-weight: 700;");
    } else {
        lblWelcome->setText("Panel de Control: Gestione su torneo activo.");
        lblWelcome->setStyleSheet("color: #0071e3; font-weight: 700;");
    }
}

void app::onPlayersClicked()
{
    QMessageBox::information(this, "Jugadores", "Abrir formulario de Jugadores (no implementado aún)");
}

void app::onTeamsClicked()
{
    QMessageBox::information(this, "Equipos", "Abrir formulario de Equipos (no implementado aún)");
}

void app::onTournamentsClicked()
{
    QMessageBox::information(this, "Torneos", "Abrir formulario de Torneos (no implementado aún)");
}

void app::onMatchesClicked()
{
    QMessageBox::information(this, "Partidos", "Use el botón Partidos para abrir el diálogo de Partidos");
}

