#include "dialogs/tournamentsdialog.h"
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
"QPushButton:hover { background:#0077ed; } QPushButton:pressed { background:#0062c3; }"
"QPushButton:disabled { background:#b3d4f5; }";
static const char* SS_SEC =
"QPushButton { background:#fbfbfd; color:#1d1d1f; border:1px solid #e6e6ea;"
"  border-radius:9px; padding:8px 18px; font-size:13px; font-weight:600; }"
"QPushButton:hover { background:#e8e8ed; } QPushButton:disabled { color:#b0b0b8; }";
static const char* SS_DANGER =
"QPushButton { background:#ff3b30; color:white; border:none; border-radius:9px;"
"  padding:8px 18px; font-size:13px; font-weight:600; }"
"QPushButton:hover { background:#ff453a; } QPushButton:disabled { background:#ffb3af; }";

tournamentsDialog::tournamentsDialog(LogTournament* logTournament, QWidget* parent)
    : QDialog(parent), logTournament_(logTournament)
{
    setWindowTitle("Gestionar Torneos");
    setMinimumSize(540, 420);
    resize(560, 440);
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
    QLabel* ico = new QLabel("T");
    ico->setFixedSize(40, 40);
    ico->setAlignment(Qt::AlignCenter);
    ico->setStyleSheet("background:rgba(0,113,227,0.12); border-radius:10px;"
        "font-size:16px; font-weight:700; color:#0071e3;");
    QVBoxLayout* titleCol = new QVBoxLayout();
    titleCol->setSpacing(1);
    QLabel* lTitle = new QLabel("Torneos");
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
    edtName_->setPlaceholderText("Nombre del torneo...");
    edtName_->setStyleSheet(SS_EDIT);
    QPushButton* btnAdd = new QPushButton("Agregar");
    btnAdd->setStyleSheet(SS_PRIMARY);
    btnAdd->setFixedHeight(36);
    addL->addWidget(edtName_, 1);
    addL->addWidget(btnAdd);
    bodyL->addWidget(addBar);

    // Lista
    listTournaments_ = new QListWidget();
    listTournaments_->setStyleSheet(SS_LIST);
    listTournaments_->setSelectionMode(QAbstractItemView::SingleSelection);
    bodyL->addWidget(listTournaments_, 1);

    // Fila fase
    phaseRow_ = new QWidget();
    phaseRow_->setStyleSheet("background:#ffffff; border:1px solid #e6e6ea; border-radius:12px;");
    QHBoxLayout* phL = new QHBoxLayout(phaseRow_);
    phL->setContentsMargins(12, 8, 12, 8);
    phL->setSpacing(8);
    QLabel* lPhase = new QLabel("Fase:");
    lPhase->setStyleSheet("font-size:12px; font-weight:600; color:#1d1d1f;");
    cmbPhase_ = new QComboBox();
    cmbPhase_->addItems({ "Registro", "Grupos", "Eliminacion", "Finalizado" });
    cmbPhase_->setStyleSheet(SS_COMBO);
    QPushButton* btnPhase = new QPushButton("Cambiar");
    btnPhase->setStyleSheet(SS_SEC);
    phL->addWidget(lPhase);
    phL->addWidget(cmbPhase_, 1);
    phL->addWidget(btnPhase);
    bodyL->addWidget(phaseRow_);
    phaseRow_->hide();

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

    connect(btnAdd, &QPushButton::clicked, this, &tournamentsDialog::onAddClicked);
    connect(btnEdit_, &QPushButton::clicked, this, &tournamentsDialog::onEditClicked);
    connect(btnDelete_, &QPushButton::clicked, this, &tournamentsDialog::onDeleteClicked);
    connect(btnRefresh, &QPushButton::clicked, this, &tournamentsDialog::onRefresh);
    connect(edtName_, &QLineEdit::returnPressed, this, &tournamentsDialog::onAddClicked);
    connect(btnPhase, &QPushButton::clicked, this, &tournamentsDialog::onChangePhaseClicked);
    connect(listTournaments_, &QListWidget::itemSelectionChanged, this, [this]() {
        bool sel = !listTournaments_->selectedItems().isEmpty();
        btnEdit_->setEnabled(sel);
        btnDelete_->setEnabled(sel);
        phaseRow_->setVisible(sel);
        if (sel) {
            int idx = cmbPhase_->findText(
                listTournaments_->currentItem()->data(Qt::UserRole + 1).toString());
            if (idx >= 0) cmbPhase_->setCurrentIndex(idx);
        }
        });

    loadTournaments();
}

tournamentsDialog::~tournamentsDialog() {}
void tournamentsDialog::ensureUi() {}

void tournamentsDialog::loadTournaments()
{
    listTournaments_->clear();
    auto res = logTournament_->list();
    for (const auto& t : res.data) {
        QString name = QString::fromStdString(t.name);
        QString phase = QString::fromStdString(t.phase);
        QListWidgetItem* item = new QListWidgetItem(
            QString("%1  —  %2").arg(name, phase), listTournaments_);
        item->setData(Qt::UserRole, t.id);
        item->setData(Qt::UserRole + 1, phase);
        listTournaments_->addItem(item);
    }
    if (lblSubtitle_)
        lblSubtitle_->setText(QString("%1 torneo(s)").arg(res.data.size()));
}

void tournamentsDialog::onAddClicked()
{
    QString name = edtName_->text().trimmed();
    if (name.isEmpty()) { edtName_->setFocus(); return; }
    auto res = logTournament_->insert(name.toStdString());
    if (res.code >= 0) { edtName_->clear(); loadTournaments(); }
    else QMessageBox::warning(this, "Error", QString::fromStdString(res.message));
}

void tournamentsDialog::onEditClicked()
{
    QListWidgetItem* item = listTournaments_->currentItem();
    if (!item) return;
    int id = item->data(Qt::UserRole).toInt();
    QString current = item->text().split("  —  ").first();
    bool ok;
    QString newName = QInputDialog::getText(this, "Editar torneo",
        "Nuevo nombre:", QLineEdit::Normal, current, &ok);
    if (ok && !newName.trimmed().isEmpty()) {
        auto res = logTournament_->updateName(id, newName.trimmed().toStdString());
        if (res.code >= 0) loadTournaments();
        else QMessageBox::warning(this, "Error", QString::fromStdString(res.message));
    }
}

void tournamentsDialog::onDeleteClicked()
{
    QListWidgetItem* item = listTournaments_->currentItem();
    if (!item) return;
    if (QMessageBox::warning(this, "Eliminar torneo",
        "¿Eliminar este torneo? La acción no se puede deshacer.",
        QMessageBox::Yes | QMessageBox::Cancel) == QMessageBox::Yes) {
        auto res = logTournament_->eliminar(item->data(Qt::UserRole).toInt());
        if (res.code >= 0) loadTournaments();
        else QMessageBox::warning(this, "Error", QString::fromStdString(res.message));
    }
}

void tournamentsDialog::onRefresh() { loadTournaments(); }

void tournamentsDialog::onChangePhaseClicked()
{
    QListWidgetItem* item = listTournaments_->currentItem();
    if (!item || !cmbPhase_) return;
    auto res = logTournament_->updatePhase(
        item->data(Qt::UserRole).toInt(),
        cmbPhase_->currentText().toStdString());
    if (res.code >= 0) loadTournaments();
    else QMessageBox::warning(this, "Error", QString::fromStdString(res.message));
}
