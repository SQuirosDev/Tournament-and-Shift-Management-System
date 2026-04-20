#include "dialogs/petitionsdialog.h"
#include <QtWidgets/QApplication>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QListWidgetItem>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QWidget>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QInputDialog>
#include <QtWidgets/QFrame>

static const char* SP_HEADER = "background:#ffffff;border-bottom:1px solid #e0e0e8;";
static const char* SP_BODY = "background:#f5f5f7;";
static const char* SP_BAR = "background:#ffffff;border:1px solid #e0e0e8;border-radius:12px;";
static const char* SP_ICO = "background:rgba(0,113,227,0.10);border-radius:10px;font-size:15px;font-weight:700;color:#0071e3;";
static const char* SP_PRIMARY = "QPushButton{background:#0071e3;color:#fff;border:none;border-radius:9px;padding:8px 18px;font-size:13px;font-weight:600;}QPushButton:hover{background:#0077ed;}QPushButton:disabled{background:#b3d4f5;}";
static const char* SP_SEC = "QPushButton{background:#ffffff;color:#1d1d1f;border:1px solid #d2d2d7;border-radius:9px;padding:8px 18px;font-size:13px;font-weight:600;}QPushButton:hover{background:#f0f0f5;}QPushButton:disabled{color:#b0b0b8;}";
static const char* SP_DANGER = "QPushButton{background:#ff3b30;color:#fff;border:none;border-radius:9px;padding:8px 18px;font-size:13px;font-weight:600;}QPushButton:hover{background:#ff453a;}QPushButton:disabled{background:#ffb3af;}";
static const char* SP_SUCCESS = "QPushButton{background:#28a745;color:#fff;border:none;border-radius:9px;padding:8px 18px;font-size:13px;font-weight:600;}QPushButton:hover{background:#218838;}QPushButton:disabled{background:#a3d9a5;}";

petitionsDialog::petitionsDialog(LogPetition* logPetition, QWidget* parent)
    : QDialog(parent), logPetition_(logPetition)
{
    setWindowTitle("Gestionar Peticiones");
    setFixedSize(620, 540);

    QVBoxLayout* root = new QVBoxLayout(this);
    root->setSpacing(0);
    root->setContentsMargins(0, 0, 0, 0);

    // ── Header ────────────────────────────────────────────────────────────────
    QWidget* hdr = new QWidget();
    hdr->setStyleSheet(SP_HEADER);
    QHBoxLayout* hL = new QHBoxLayout(hdr);
    hL->setContentsMargins(22, 14, 22, 14);
    hL->setSpacing(12);

    QLabel* ico = new QLabel("P");
    ico->setFixedSize(38, 38);
    ico->setAlignment(Qt::AlignCenter);
    ico->setStyleSheet(SP_ICO);

    QVBoxLayout* tc = new QVBoxLayout();
    tc->setSpacing(1);
    QLabel* lt = new QLabel("Peticiones");
    lt->setStyleSheet("font-size:14px;font-weight:700;color:#1d1d1f;");
    lblSubtitle_ = new QLabel();
    lblSubtitle_->setStyleSheet("font-size:11px;color:#86868b;");
    tc->addWidget(lt);
    tc->addWidget(lblSubtitle_);

    hL->addWidget(ico);
    hL->addLayout(tc);
    hL->addStretch();
    root->addWidget(hdr);

    // ── Body ──────────────────────────────────────────────────────────────────
    QWidget* body = new QWidget();
    body->setStyleSheet(SP_BODY);
    QVBoxLayout* bL = new QVBoxLayout(body);
    bL->setContentsMargins(18, 14, 18, 14);
    bL->setSpacing(10);

    // Tabs
    QTabWidget* tabs = new QTabWidget();

    // ── Tab 1: Lista de peticiones pendientes ─────────────────────────────────
    QWidget* tabList = new QWidget();
    QVBoxLayout* tlL = new QVBoxLayout(tabList);
    tlL->setContentsMargins(12, 12, 12, 12);
    tlL->setSpacing(10);

    // Banner "siguiente en cola"
    QWidget* queueBar = new QWidget();
    queueBar->setStyleSheet(SP_BAR);
    QHBoxLayout* qL = new QHBoxLayout(queueBar);
    qL->setContentsMargins(14, 10, 14, 10);
    qL->setSpacing(8);
    QLabel* qIco = new QLabel("⏭");
    qIco->setStyleSheet("font-size:14px;");
    QLabel* qTitle = new QLabel("Siguiente en cola:");
    qTitle->setStyleSheet("font-size:12px;font-weight:600;color:#1d1d1f;");
    lblNextInQueue_ = new QLabel("—");
    lblNextInQueue_->setStyleSheet("font-size:12px;color:#0071e3;font-weight:600;");
    lblNextInQueue_->setWordWrap(true);
    qL->addWidget(qIco);
    qL->addWidget(qTitle);
    qL->addWidget(lblNextInQueue_, 1);
    tlL->addWidget(queueBar);

    // Lista
    listPetitions_ = new QListWidget();
    listPetitions_->setSelectionMode(QAbstractItemView::SingleSelection);
    tlL->addWidget(listPetitions_, 1);

    // Toolbar de acciones
    QHBoxLayout* actL = new QHBoxLayout();
    actL->setSpacing(8);
    btnRespond_ = new QPushButton("Responder");
    btnRespond_->setStyleSheet(SP_SUCCESS);
    btnDelete_ = new QPushButton("Eliminar");
    btnDelete_->setStyleSheet(SP_DANGER);
    QPushButton* btnR = new QPushButton("Actualizar");
    btnR->setStyleSheet(SP_SEC);
    btnRespond_->setEnabled(false);
    btnDelete_->setEnabled(false);
    actL->addWidget(btnRespond_);
    actL->addWidget(btnDelete_);
    actL->addStretch();
    actL->addWidget(btnR);
    tlL->addLayout(actL);

    tabs->addTab(tabList, "Peticiones pendientes");

    // ── Tab 2: Nueva petición ─────────────────────────────────────────────────
    QWidget* tabNew = new QWidget();
    QVBoxLayout* tnL = new QVBoxLayout(tabNew);
    tnL->setContentsMargins(16, 16, 16, 16);
    tnL->setSpacing(10);

    // Solicitante
    QWidget* reqBar = new QWidget();
    reqBar->setStyleSheet(SP_BAR);
    QHBoxLayout* rL = new QHBoxLayout(reqBar);
    rL->setContentsMargins(12, 8, 12, 8);
    rL->setSpacing(8);
    QLabel* lReq = new QLabel("Solicitante:");
    lReq->setStyleSheet("font-size:12px;font-weight:600;color:#86868b;");
    lReq->setFixedWidth(90);
    edtRequester_ = new QLineEdit();
    edtRequester_->setPlaceholderText("Nombre del solicitante...");
    edtRequester_->setStyleSheet("QLineEdit{background:#ffffff;color:#1d1d1f;border:none;font-size:12px;}");
    rL->addWidget(lReq);
    rL->addWidget(edtRequester_, 1);
    tnL->addWidget(reqBar);

    // Tipo
    QWidget* typeBar = new QWidget();
    typeBar->setStyleSheet(SP_BAR);
    QHBoxLayout* tyL = new QHBoxLayout(typeBar);
    tyL->setContentsMargins(12, 8, 12, 8);
    tyL->setSpacing(8);
    QLabel* lType = new QLabel("Tipo:");
    lType->setStyleSheet("font-size:12px;font-weight:600;color:#86868b;");
    lType->setFixedWidth(90);
    cmbType_ = new QComboBox();
    cmbType_->addItem("Inscripcion");
    cmbType_->addItem("Consulta");
    cmbType_->addItem("Apelacion");
    cmbType_->setStyleSheet("QComboBox{background:#ffffff;color:#1d1d1f;border:none;font-size:12px;}QComboBox QAbstractItemView{background:#ffffff;color:#1d1d1f;}");
    tyL->addWidget(lType);
    tyL->addWidget(cmbType_, 1);
    tnL->addWidget(typeBar);

    // Descripción
    QWidget* descBar = new QWidget();
    descBar->setStyleSheet(SP_BAR);
    QVBoxLayout* dL = new QVBoxLayout(descBar);
    dL->setContentsMargins(12, 10, 12, 10);
    dL->setSpacing(6);
    QLabel* lDesc = new QLabel("Descripción:");
    lDesc->setStyleSheet("font-size:12px;font-weight:600;color:#86868b;");
    edtDescription_ = new QTextEdit();
    edtDescription_->setPlaceholderText("Describe la petición...");
    edtDescription_->setFixedHeight(100);
    edtDescription_->setStyleSheet("QTextEdit{background:#ffffff;color:#1d1d1f;border:none;font-size:12px;padding:4px;}");
    dL->addWidget(lDesc);
    dL->addWidget(edtDescription_);
    tnL->addWidget(descBar);

    tnL->addStretch();

    // Botón enviar
    QHBoxLayout* sbL = new QHBoxLayout();
    QPushButton* btnAdd = new QPushButton("Enviar petición");
    btnAdd->setStyleSheet(SP_PRIMARY);
    btnAdd->setFixedHeight(38);
    sbL->addStretch();
    sbL->addWidget(btnAdd);
    tnL->addLayout(sbL);

    tabs->addTab(tabNew, "Nueva petición");

    bL->addWidget(tabs, 1);
    root->addWidget(body, 1);

    // ── Conexiones ────────────────────────────────────────────────────────────
    connect(btnAdd, &QPushButton::clicked, this, &petitionsDialog::onAddClicked);
    connect(btnRespond_, &QPushButton::clicked, this, &petitionsDialog::onRespondClicked);
    connect(btnDelete_, &QPushButton::clicked, this, &petitionsDialog::onDeleteClicked);
    connect(btnR, &QPushButton::clicked, this, &petitionsDialog::onRefresh);
    connect(listPetitions_, &QListWidget::itemSelectionChanged, this, [this]() {
        bool s = !listPetitions_->selectedItems().isEmpty();
        btnRespond_->setEnabled(s);
        btnDelete_->setEnabled(s);
        });

    loadPetitions();
}

petitionsDialog::~petitionsDialog() {}

// ── Carga ─────────────────────────────────────────────────────────────────────

void petitionsDialog::loadPetitions()
{
    listPetitions_->clear();
    auto res = logPetition_->listPending();
    for (const auto& p : res.data) {
        QString line = QString("[%1]  %2  —  %3")
            .arg(QString::fromStdString(p.type))
            .arg(QString::fromStdString(p.requesterName))
            .arg(QString::fromStdString(p.description).left(50));
        QListWidgetItem* item = new QListWidgetItem(line, listPetitions_);
        item->setData(Qt::UserRole, p.id);
        item->setData(Qt::UserRole + 1, QString::fromStdString(p.requesterName));
        item->setData(Qt::UserRole + 2, QString::fromStdString(p.type));
        listPetitions_->addItem(item);
    }

    int total = static_cast<int>(res.data.size());
    if (lblSubtitle_)
        lblSubtitle_->setText(QString("%1 petición(es) pendiente(s)").arg(total));

    updateNextInQueue();
    btnRespond_->setEnabled(false);
    btnDelete_->setEnabled(false);
}

void petitionsDialog::updateNextInQueue()
{
    auto res = logPetition_->peekNext();
    if (res.data.empty()) {
        lblNextInQueue_->setText("No hay peticiones pendientes");
        return;
    }
    const auto& p = res.data[0];
    lblNextInQueue_->setText(
        QString("[%1]  %2")
        .arg(QString::fromStdString(p.type))
        .arg(QString::fromStdString(p.requesterName)));
}

// ── Slots ─────────────────────────────────────────────────────────────────────

void petitionsDialog::onAddClicked()
{
    QString requester = edtRequester_->text().trimmed();
    QString type = cmbType_->currentText();
    QString desc = edtDescription_->toPlainText().trimmed();

    if (requester.isEmpty()) {
        edtRequester_->setFocus();
        QMessageBox::warning(this, "Campo requerido", "El nombre del solicitante no puede estar vacío.");
        return;
    }

    auto r = logPetition_->insert(
        requester.toStdString(),
        type.toStdString(),
        desc.toStdString()
    );

    if (r.code >= 0) {
        edtRequester_->clear();
        edtDescription_->clear();
        cmbType_->setCurrentIndex(0);
        loadPetitions();
        QMessageBox::information(this, "Petición enviada", "La petición fue registrada correctamente.");
    }
    else {
        QMessageBox::warning(this, "Error", QString::fromStdString(r.message));
    }
}

void petitionsDialog::onRespondClicked()
{
    auto* item = listPetitions_->currentItem();
    if (!item) return;

    int     id = item->data(Qt::UserRole).toInt();
    QString name = item->data(Qt::UserRole + 1).toString();
    QString type = item->data(Qt::UserRole + 2).toString();

    bool ok;
    QString response = QInputDialog::getText(
        this,
        "Responder petición",
        QString("Respuesta para [%1] %2:").arg(type, name),
        QLineEdit::Normal, "", &ok
    );

    if (!ok || response.trimmed().isEmpty()) return;

    auto r = logPetition_->update(id, response.trimmed().toStdString());
    if (r.code >= 0) {
        loadPetitions();
        QMessageBox::information(this, "Petición atendida", "La petición fue marcada como atendida.");
    }
    else {
        QMessageBox::warning(this, "Error", QString::fromStdString(r.message));
    }
}

void petitionsDialog::onDeleteClicked()
{
    auto* item = listPetitions_->currentItem();
    if (!item) return;

    if (QMessageBox::warning(this, "Eliminar petición",
        "¿Eliminar esta petición? Esta acción no se puede deshacer.",
        QMessageBox::Yes | QMessageBox::Cancel) == QMessageBox::Yes)
    {
        auto r = logPetition_->eliminar(item->data(Qt::UserRole).toInt());
        if (r.code >= 0)
            loadPetitions();
        else
            QMessageBox::warning(this, "Error", QString::fromStdString(r.message));
    }
}

void petitionsDialog::onRefresh()
{
    loadPetitions();
}

