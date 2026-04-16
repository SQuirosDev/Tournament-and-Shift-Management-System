#pragma once

static const char* DLG_SS_DIALOG =
"QDialog { background:#f5f5f7; }";

static const char* DLG_SS_COMBO =
"QComboBox {"
"  background:#ffffff; border:1px solid #d2d2d7; border-radius:10px;"
"  padding:8px 12px; font-size:13px; color:#1d1d1f; }"
"QComboBox:focus { border:2px solid #0071e3; }"
"QComboBox::drop-down { border:none; width:20px; }"
"QComboBox QAbstractItemView {"          // ← el popup desplegado
"  background:#ffffff; color:#1d1d1f;"
"  border:1px solid #d2d2d7; border-radius:8px;"
"  selection-background-color:#e8f2ff; selection-color:#0071e3;"
"  outline:none; }";

static const char* DLG_SS_EDIT =
"QLineEdit {"
"  background:#ffffff; border:1px solid #d2d2d7; border-radius:10px;"
"  padding:9px 12px; font-size:13px; color:#1d1d1f; }"
"QLineEdit:focus { border:2px solid #0071e3; }";

static const char* DLG_SS_LIST =
"QListWidget {"
"  border:1px solid #d2d2d7; border-radius:12px;"
"  background:#ffffff; padding:4px; outline:none; color:#1d1d1f; }"
"QListWidget::item {"
"  border-radius:8px; padding:10px 12px; margin:1px 0; color:#1d1d1f; }"
"QListWidget::item:selected { background:#e8f2ff; color:#0071e3; }"
"QListWidget::item:hover:!selected { background:#f0f0f5; color:#1d1d1f; }";

static const char* DLG_SS_TABLE =
"QTableWidget {"
"  border:1px solid #d2d2d7; border-radius:12px;"
"  background:#ffffff; gridline-color:#f0f0f5; color:#1d1d1f;"
"  outline:none; font-size:12px; }"
"QTableWidget::item { padding:8px 10px; color:#1d1d1f; }"
"QTableWidget::item:selected { background:#e8f2ff; color:#0071e3; }"
"QHeaderView::section {"
"  background:#f5f5f7; color:#1d1d1f; font-weight:600; font-size:12px;"
"  border:none; border-bottom:1px solid #d2d2d7; padding:8px 10px; }";

static const char* DLG_SS_BTN_PRIMARY =
"QPushButton {"
"  background:#0071e3; color:#ffffff; border:none;"
"  border-radius:9px; padding:8px 18px; font-size:13px; font-weight:600; }"
"QPushButton:hover  { background:#0077ed; }"
"QPushButton:pressed{ background:#0062c3; }"
"QPushButton:disabled{ background:#b3d4f5; color:#ffffff; }";

static const char* DLG_SS_BTN_SEC =
"QPushButton {"
"  background:#ffffff; color:#1d1d1f; border:1px solid #d2d2d7;"
"  border-radius:9px; padding:8px 18px; font-size:13px; font-weight:600; }"
"QPushButton:hover  { background:#f0f0f5; }"
"QPushButton:pressed{ background:#e0e0e8; }"
"QPushButton:disabled{ color:#b0b0b8; background:#f5f5f7; }";

static const char* DLG_SS_BTN_DANGER =
"QPushButton {"
"  background:#ff3b30; color:#ffffff; border:none;"
"  border-radius:9px; padding:8px 18px; font-size:13px; font-weight:600; }"
"QPushButton:hover  { background:#ff453a; }"
"QPushButton:pressed{ background:#d63029; }"
"QPushButton:disabled{ background:#ffb3af; color:#ffffff; }";

static const char* DLG_SS_BTN_SUCCESS =
"QPushButton {"
"  background:#34c759; color:#ffffff; border:none;"
"  border-radius:9px; padding:8px 18px; font-size:13px; font-weight:600; }"
"QPushButton:hover  { background:#30d158; }"
"QPushButton:disabled{ background:#a8e6b8; color:#ffffff; }";

static const char* DLG_SS_BTN_SIM =
"QPushButton {"
"  background:#f5f5f7; color:#1d1d1f; border:1px solid #d2d2d7;"
"  border-radius:8px; padding:6px 12px; font-size:12px; font-weight:600; }"
"QPushButton:hover  { background:#e8e8ed; }"
"QPushButton:disabled{ color:#b0b0b8; }";

// Tamaño estándar de todas las ventanas de diálogo
static const int DLG_W = 580;
static const int DLG_H = 500;
static const int DLG_W_WIDE = 720;  // partidos (
static const int DLG_H_WIDE = 560;
