#include "HalsWindow.h"

#include <QMessageBox>
#include <QPixmap>

#include "ui_HalsWindow.h"

HalsWindow::HalsWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::HalsWindow) {
    ui->setupUi(this);
    setupGui();
}

HalsWindow::~HalsWindow() { delete ui; }

void HalsWindow::on_pushButtonQuit_clicked() {
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Подтверждение", "Завершить работу приложения?",
        QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        qApp->quit();
    }
}

void HalsWindow::setupGui() {
    setWindowFlags(Qt::FramelessWindowHint);
    ui->labelLogo->setPixmap(QPixmap(":/Icons/hals_128.png"));
    applyStyleSheet();
}

void HalsWindow::applyStyleSheet() {
    QString styleSheet = R"(
        /* Основное окно и виджеты */
        QMainWindow, QWidget {
            background-color: #FFF2CC;   /* светло-жёлтый фон */
            color: #1E1E1E;              /* почти чёрный текст */
            font-family: "Arial", "Helvetica", sans-serif;
            font-size: 12pt;
        }
        QLabel {
            color: #1E1E1E;
        }
        QGroupBox {
            border: 1px solid #CCAA66;
            border-radius: 5px;
            margin-top: 0.5em;
            font-size: 14pt;
            font-weight: bold;
            color: #1E1E1E;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 3px 0 3px;
            background-color: #FFF2CC;
        }
        QPushButton {
            background-color: #FFDD88;
            border: 1px solid #CCAA66;
            border-radius: 4px;
            padding: 8px;
            font-size: 14pt;
            font-weight: bold;
            min-height: 48px;
            color: #1E1E1E;
        }
        QPushButton:hover {
            background-color: #FFCC66;
        }
        QPushButton:pressed {
            background-color: #EEBB55;
        }
        /* Специальный стиль для кнопки выхода (красный для привлечения внимания) */
        QPushButton#pushButtonQuit {
            background-color: #FF8888;
            border-color: #AA6666;
            color: #1E1E1E;
        }
        QPushButton#pushButtonQuit:hover {
            background-color: #FFAAAA;
        }
        /* Поля ввода */
        QLineEdit, QSpinBox, QDoubleSpinBox, QComboBox {
            background-color: #FFFFFF;
            border: 1px solid #CCAA66;
            border-radius: 3px;
            padding: 4px;
            color: #1E1E1E;
        }
        QLineEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus, QComboBox:focus {
            border-color: #AA8844;
        }
        QStatusBar {
            background-color: #FFF2CC;
            color: #1E1E1E;
        }

        /* Стилизация диалогового окна QMessageBox и его кнопок */
        QMessageBox {
            background-color: #FFF2CC;
            font-size: 12pt;
        }
        QMessageBox QPushButton {
            min-width: 120px;
            min-height: 48px;
            font-size: 14pt;
            padding: 8px 12px;
            background-color: #FFDD88;
            border: 1px solid #CCAA66;
            border-radius: 4px;
            color: #1E1E1E;
        }
        QMessageBox QPushButton:hover {
            background-color: #FFCC66;
        }
        QMessageBox QPushButton:pressed {
            background-color: #EEBB55;
        }
    )";
    qApp->setStyleSheet(styleSheet);
}
