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

static const QString SP_HEADER = "background:#ffffff;border-bottom:1px solid #e0e0e8;";
static const QString SP_BODY = "background:#f5f5f7;";
static const QString SP_BAR = "background:#ffffff;border:1px solid #e0e0e8;border-radius:12px;";
static const QString SP_ICO = "background:rgba(0,113,227,0.10);border-radius:10px;font-size:15px;font-weight:700;color:#0071e3;";
static const QString SP_PRIMARY = "QPushButton{background:#0071e3;color:#fff;border:none;border-radius:9px;padding:8px 18px;font-size:13px;font-weight:600;}QPushButton:hover{background:#0077ed;}QPushButton:disabled{background:#b3d4f5;}";
static const QString SP_SEC = "QPushButton{background:#ffffff;color:#1d1d1f;border:1px solid #d2d2d7;border-radius:9px;padding:8px 18px;font-size:13px;font-weight:600;}QPushButton:hover{background:#f0f0f5;}QPushButton:disabled{color:#b0b0b8;}";
static const QString SP_DANGER = "QPushButton{background:#ff3b30;color:#fff;border:none;border-radius:9px;padding:8px 18px;font-size:13px;font-weight:600;}QPushButton:hover{background:#ff453a;}QPushButton:disabled{background:#ffb3af;}";
static const QString SP_SUCCESS = "QPushButton{background:#28a745;color:#fff;border:none;border-radius:9px;padding:8px 18px;font-size:13px;font-weight:600;}QPushButton:hover{background:#218838;}QPushButton:disabled{background:#a3d9a5;}";

petitionsDialog::petitionsDialog(LogPetition* logPetition, QWidget* parent)
    : QDialog(parent)
    , logPetition_(logPetition)
    , listPetitions_(nullptr)
    , lblSubtitle_(nullptr)
    , lblNextInQueue_(nullptr)
    , btnRespond_(nullptr)
    , btnDelete_(nullptr)
    , edtRequester_(nullptr)
    , cmbType_(nullptr)
    , edtDescription_(nullptr)
{
    setWindowTitle("Gestionar Peticiones");
    setFixedSize(620, 540);

    QVBoxLayout* root = new QVBoxLayout(this);
    root->setSpacing(0);
    root->setContentsMargins(0, 0, 0, 0);

    // ── Header ────────────────────────────────────────────────────────────────
    QWidget* hdr = new QWidget(this);
    hdr->setStyleSheet(SP_HEADER);
    QHBoxLayout* hL = new QHBoxLayout(hdr);
    hL->setContentsMargins(22, 14, 22, 14);
    hL->setSpacing(12);

    QLabel* ico = new QLabel("P", hdr);
    ico->setFixedSize(38, 38);
    ico->setAlignment(Qt::AlignCenter);
    ico->setStyleSheet(SP_ICO);

    QVBoxLayout* tc = new QVBoxLayout();
    tc->setSpacing(1);
    QLabel* lt = new QLabel("Peticiones", hdr);
    lt->setStyleSheet("font-size:14px;font-weight:700;color:#1d1d1f;");
    lblSubtitle_ = new QLabel("", hdr);
    lblSubtitle_->setStyleSheet("font-size:11px;color:#86868b;");
    tc->addWidget(lt);
    tc->addWidget(lblSubtitle_);

    hL->addWidget(ico);
    hL->addLayout(tc);
    hL->addStretch();
    root->addWidget(hdr);

    // ── Body ──────────────────────────────────────────────────────────────────
    QWidget* body = new QWidget(this);
    body->setStyleSheet(SP_BODY);
    QVBoxLayout* bL = new QVBoxLayout(body);
    bL->setContentsMargins(18, 14, 18, 14);
    bL->setSpacing(10);

    QTabWidget* tabs = new QTabWidget(body);

    // ── Tab 1: Peticiones pendientes ──────────────────────────────────────────
    QWidget* tabList = new QWidget();
    QVBoxLayout* tlL = new QVBoxLayout(tabList);
    tlL->setContentsMargins(12, 12, 12, 12);
    tlL->setSpacing(10);

    // Banner siguiente en cola
    QWidget* queueBar = new QWidget(tabList);
    queueBar->setStyleSheet(SP_BAR);
    QHBoxLayout* qL = new QHBoxLayout(queueBar);
    qL->setContentsMargins(14, 10, 14, 10);
    qL->setSpacing(8);
    QLabel* qIco = new QLabel(QString::fromUtf8("\u23ED"), queueBar);
    qIco->setStyleSheet("font-size:14px;");
    QLabel* qTitle = new QLabel("Siguiente en cola:", queueBar);
    qTitle->setStyleSheet("font-size:12px;font-weight:600;color:#1d1d1f;");
    lblNextInQueue_ = new QLabel("—", queueBar);
    lblNextInQueue_->setStyleSheet("font-size:12px;color:#0071e3;font-weight:600;");
    lblNextInQueue_->setWordWrap(true);
    qL->addWidget(qIco);
    qL->addWidget(qTitle);
    qL->addWidget(lblNextInQueue_, 1);
    tlL->addWidget(queueBar);

    // Lista
    listPetitions_ = new QListWidget(tabList);
    listPetitions_->setSelectionMode(QAbstractItemView::SingleSelection);
    tlL->addWidget(listPetitions_, 1);

    // Toolbar
    QHBoxLayout* actL = new QHBoxLayout();
    actL->setSpacing(8);
    btnRespond_ = new QPushButton("Responder", tabList);
    btnRespond_->setStyleSheet(SP_SUCCESS);
    btnDelete_ = new QPushButton("Eliminar", tabList);
    btnDelete_->setStyleSheet(SP_DANGER);
    QPushButton* btnR = new QPushButton("Actualizar", tabList);
    btnR->setStyleSheet(SP_SEC);
    btnRespond_->setEnabled(false);
    btnDelete_->setEnabled(false);
    actL->addWidget(btnRespond_);
    actL->addWidget(btnDelete_);
    actL->addStretch();
    actL->addWidget(btnR);
    tlL->addLayout(actL);

    connect(btnRespond_, &QPushButton::clicked, this, &petitionsDialog::onRespondClicked);
    connect(btnDelete_, &QPushButton::clicked, this, &petitionsDialog::onDeleteClicked);
    connect(btnR, &QPushButton::clicked, this, &petitionsDialog::onRefresh);
    connect(listPetitions_, &QListWidget::itemSelectionChanged, this, [this]() {
        bool sel = !listPetitions_->selectedItems().isEmpty();
        btnRespond_->setEnabled(sel);
        btnDelete_->setEnabled(sel);
        });

    tabs->addTab(tabList, "Peticiones pendientes");

    // ── Tab 2: Nueva peticion ─────────────────────────────────────────────────
    QWidget* tabNew = new QWidget();
    QVBoxLayout* tnL = new QVBoxLayout(tabNew);
    tnL->setContentsMargins(16, 16, 16, 16);
    tnL->setSpacing(10);

    // Solicitante
    QWidget* reqBar = new QWidget(tabNew);
    reqBar->setStyleSheet(SP_BAR);
    QHBoxLayout* rL = new QHBoxLayout(reqBar);
    rL->setContentsMargins(12, 8, 12, 8);
    rL->setSpacing(8);
    QLabel* lReq = new QLabel("Solicitante:", reqBar);
    lReq->setStyleSheet("font-size:12px;font-weight:600;color:#86868b;");
    lReq->setFixedWidth(90);
    edtRequester_ = new QLineEdit(reqBar);
    edtRequester_->setPlaceholderText("Nombre del solicitante...");
    rL->addWidget(lReq);
    rL->addWidget(edtRequester_, 1);
    tnL->addWidget(reqBar);

    // Tipo
    QWidget* typeBar = new QWidget(tabNew);
    typeBar->setStyleSheet(SP_BAR);
    QHBoxLayout* tyL = new QHBoxLayout(typeBar);
    tyL->setContentsMargins(12, 8, 12, 8);
    tyL->setSpacing(8);
    QLabel* lType = new QLabel("Tipo:", typeBar);
    lType->setStyleSheet("font-size:12px;font-weight:600;color:#86868b;");
    lType->setFixedWidth(90);
    cmbType_ = new QComboBox(typeBar);
    cmbType_->addItem("Inscripcion");
    cmbType_->addItem("Consulta");
    cmbType_->addItem("Apelacion");
    tyL->addWidget(lType);
    tyL->addWidget(cmbType_, 1);
    tnL->addWidget(typeBar);

    // Descripcion
    QWidget* descBar = new QWidget(tabNew);
    descBar->setStyleSheet(SP_BAR);
    QVBoxLayout* dL = new QVBoxLayout(descBar);
    dL->setContentsMargins(12, 10, 12, 10);
    dL->setSpacing(6);
    QLabel* lDesc = new QLabel("Descripcion:", descBar);
    lDesc->setStyleSheet("font-size:12px;font-weight:600;color:#86868b;");
    edtDescription_ = new QTextEdit(descBar);
    edtDescription_->setPlaceholderText("Describe la peticion...");
    edtDescription_->setFixedHeight(100);
    dL->addWidget(lDesc);
    dL->addWidget(edtDescription_);
    tnL->addWidget(descBar);

    tnL->addStretch();

    // Boton enviar
    QHBoxLayout* sbL = new QHBoxLayout();
    QPushButton* btnAdd = new QPushButton("Enviar peticion", tabNew);
    btnAdd->setStyleSheet(SP_PRIMARY);
    btnAdd->setFixedHeight(38);
    sbL->addStretch();
    sbL->addWidget(btnAdd);
    tnL->addLayout(sbL);

    connect(btnAdd, &QPushButton::clicked, this, &petitionsDialog::onAddClicked);

    tabs->addTab(tabNew, "Nueva peticion");

    bL->addWidget(tabs, 1);
    root->addWidget(body, 1);

    loadPetitions();
}

petitionsDialog::~petitionsDialog() {}

// ── Helpers ───────────────────────────────────────────────────────────────────

void petitionsDialog::loadPetitions()
{
    listPetitions_->clear();

    BackendQueryResponse<Petition> res = logPetition_->listPending();

    for (int i = 0; i < (int)res.data.size(); i++) {
        const Petition& p = res.data[i];
        QString type = QString::fromStdString(p.type);
        QString name = QString::fromStdString(p.requesterName);
        QString desc = QString::fromStdString(p.description);
        QString line = "[" + type + "]  " + name + "  -  " + desc.left(50);

        QListWidgetItem* item = new QListWidgetItem(line, listPetitions_);
        item->setData(Qt::UserRole, p.id);
        item->setData(Qt::UserRole + 1, name);
        item->setData(Qt::UserRole + 2, type);
        listPetitions_->addItem(item);
    }

    int total = (int)res.data.size();
    lblSubtitle_->setText(QString::number(total) + " peticion(es) pendiente(s)");

    updateNextInQueue();
    btnRespond_->setEnabled(false);
    btnDelete_->setEnabled(false);
}

void petitionsDialog::updateNextInQueue()
{
    BackendQueryResponse<Petition> res = logPetition_->peekNext();
    if (res.data.empty()) {
        lblNextInQueue_->setText("No hay peticiones pendientes");
        return;
    }
    const Petition& p = res.data[0];
    QString type = QString::fromStdString(p.type);
    QString name = QString::fromStdString(p.requesterName);
    lblNextInQueue_->setText("[" + type + "]  " + name);
}

// ── Slots ─────────────────────────────────────────────────────────────────────

void petitionsDialog::onAddClicked()
{
    QString requester = edtRequester_->text().trimmed();
    QString type = cmbType_->currentText();
    QString desc = edtDescription_->toPlainText().trimmed();

    if (requester.isEmpty()) {
        edtRequester_->setFocus();
        QMessageBox::warning(this, "Campo requerido", "El nombre del solicitante no puede estar vacio.");
        return;
    }

    BackendResponse r = logPetition_->insert(
        requester.toStdString(),
        type.toStdString(),
        desc.toStdString()
    );

    if (r.code >= 0) {
        edtRequester_->clear();
        edtDescription_->clear();
        cmbType_->setCurrentIndex(0);
        loadPetitions();
        QMessageBox::information(this, "Peticion enviada", "La peticion fue registrada correctamente.");
    }
    else {
        QMessageBox::warning(this, "Error", QString::fromStdString(r.message));
    }
}

void petitionsDialog::onRespondClicked()
{
    QListWidgetItem* item = listPetitions_->currentItem();
    if (!item) return;

    int     id = item->data(Qt::UserRole).toInt();
    QString name = item->data(Qt::UserRole + 1).toString();
    QString type = item->data(Qt::UserRole + 2).toString();

    QString label = "Respuesta para [" + type + "] " + name + ":";

    bool ok = false;
    QString response = QInputDialog::getText(
        this,
        "Responder peticion",
        label,
        QLineEdit::Normal,
        "",
        &ok
    );

    if (!ok || response.trimmed().isEmpty()) return;

    BackendResponse r = logPetition_->update(id, response.trimmed().toStdString());
    if (r.code >= 0) {
        loadPetitions();
        QMessageBox::information(this, "Peticion atendida", "La peticion fue marcada como atendida.");
    }
    else {
        QMessageBox::warning(this, "Error", QString::fromStdString(r.message));
    }
}

void petitionsDialog::onDeleteClicked()
{
    QListWidgetItem* item = listPetitions_->currentItem();
    if (!item) return;

    int id = item->data(Qt::UserRole).toInt();

    QMessageBox::StandardButton btn = QMessageBox::warning(
        this,
        "Eliminar peticion",
        "Eliminar esta peticion?",
        QMessageBox::Yes | QMessageBox::Cancel
    );

    if (btn != QMessageBox::Yes) return;

    BackendResponse r = logPetition_->eliminar(id);
    if (r.code >= 0) {
        loadPetitions();
    }
    else {
        QMessageBox::warning(this, "Error", QString::fromStdString(r.message));
    }
}

void petitionsDialog::onRefresh()
{
    loadPetitions();
}
