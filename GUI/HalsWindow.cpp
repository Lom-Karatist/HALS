#include "HalsWindow.h"

#include <QMessageBox>
#include <QPixmap>
#include <QStyle>

#include "ui_HalsWindow.h"

HalsWindow::HalsWindow(QWidget* parent)
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
    ui->labelLogo->setPixmap(QPixmap(":/Icons/hals_image.png"));
    ui->labelGps->setPixmap(QPixmap(":/Icons/satellite.png"));
    applyStyleSheet();

    ui->pushButtonSaveParameters->setIcon(QIcon(":/Icons/save.png"));
    ui->pushButtonSaveParameters->setIconSize(QSize(24, 24));

    ui->pushButtonSettings->setIcon(QIcon(":/Icons/setting.png"));
    ui->pushButtonSettings->setIconSize(QSize(24, 24));

    ui->pushButtonStartStop->setIcon(QIcon(":/Icons/play.png"));
    ui->pushButtonStartStop->setIconSize(QSize(24, 24));

    QList<QPushButton*> buttons = findChildren<QPushButton*>();
    for (QPushButton* btn : buttons) {
        btn->style()->polish(btn);
    }

    addStatusIndicators();
}

void HalsWindow::applyStyleSheet() {
    QString styleSheet = R"(
        /* Основное окно и виджеты */
        QMainWindow, QWidget {
            background-color: #1a1a1a;   /* zinc-900 */
            color: #ffffff;
            font-size: 14pt;
            font-family: "Geologica Thin", "Arial", "Helvetica", sans-serif;
        }
        QLabel {
            color: #ffffff;
        }
        QGroupBox {
            border: 1px solid #CCAA66;
            border-radius: 5px;
            margin-top: 0.5em;
            font-size: 15pt;
            font-weight: bold;
            color: #ffffff;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 3px 0 3px;
            background-color: #1a1a1a;
        }

        /* ---- Базовый стиль для всех кнопок ---- */
        QPushButton {
            background-color: #27272a;   /* zinc-800 */
            border: 2px solid #3f3f46;   /* zinc-600 */
            border-radius: 12px;
            padding: 8px;
            font-size: 12pt;
            font-weight: 500;
            min-height: 48px;
            color: #ffffff;
            spacing: 8px;
        }
        QPushButton:hover {
            background-color: #3f3f46;
        }
        QPushButton:pressed {
            background-color: #52525b;
        }

        /* ---- Специальные кнопки ---- */
        QPushButton#pushButtonSaveParameters {
            background-color: #2563eb;   /* blue-600 */
            border-color: #3b82f6;
            padding: 8px;
            font-size: 12pt;
        }
        QPushButton#pushButtonSaveParameters:pressed {
            background-color: #1d4ed8;
        }
        QPushButton#pushButtonStartStop {
            background-color: #d97706;   /* amber-600 */
            border-color: #eab308;
            font-size: 12pt;
        }
        QPushButton#pushButtonStartStop:pressed {
            background-color: #b45309;
        }
        /* Состояние "активно" (checked) – красный фон */
        QPushButton#pushButtonStartStop:checked {
            background-color: #E7000B;   /* ярко-красный */
            border-color: #C30009;
        }
        QPushButton#pushButtonStartStop:checked:pressed {
            background-color: #C30009;   /* тёмно-красный при нажатии */
        }

        /* Кнопка выхода (иконка) – прозрачная */
        QPushButton#pushButtonQuit {
            background-color: transparent;
            border: none;
            background-image: url(:/Icons/power.png);
            background-repeat: no-repeat;
            background-position: center;
        }
        QPushButton#pushButtonQuit:hover,
        QPushButton#pushButtonQuit:pressed {
            background-color: transparent; /* убираем эффект наведения */
        }

        /* ---- Верхняя панель (статус) ---- */
        QWidget#upperStatusWidget {
            background-color: rgba(128, 128, 128, 64);
            border-bottom: 2px solid #fe9a00;
            height: 48px;
        }
        QWidget#upperStatusWidget QLabel {
            background-color: transparent;
            border: none;
            padding: 0;
            font-weight: bold;
            font-size: 13pt;
            color: #ffffff;
        }

        /* ---- Верхний и нижний виджеты ---- */
        QWidget#upperWidget {
            border-bottom: 2px solid #3f3f46;
        }
        QWidget#downWidget {
            border-top: 2px solid #3f3f46;
        }


        QMessageBox {
            background-color: #1a1a1a;
            font-size: 13pt;
        }
        QMessageBox QPushButton {
            font-size: 14pt;
            font-weight: bold;
            min-width: 120px;
            min-height: 48px;
            padding: 8px 12px;
        }
    )";
    qApp->setStyleSheet(styleSheet);
}

void HalsWindow::addStatusIndicators() {
    auto* usbIndicator = new StatusIndicator(this, ":/Icons/flash-drive");
    usbIndicator->setLabelText("Флеш-накопитель");

    auto* hsIndicator = new StatusIndicator(this, ":/Icons/focus");
    hsIndicator->setLabelText("Сенсор ГС");

    auto* ocIndicator = new StatusIndicator(this, ":/Icons/camera");
    ocIndicator->setLabelText("Обзорная камера");

    auto* sunIndicator = new StatusIndicator(this, ":/Icons/sun");
    sunIndicator->setIconBaseName(":/Icons/sun");
    sunIndicator->setLabelText("Сенсор освещенности");

    auto* missionIndicator = new StatusIndicator(this, ":/Icons/checklist");
    missionIndicator->setLabelText("Полётное задание");

    // Добавляем в горизонтальный лейаут centerWidget

    ui->centerHLayout->addWidget(usbIndicator);
    ui->centerHLayout->addWidget(sunIndicator);
    ui->centerHLayout->addWidget(hsIndicator);
    ui->centerHLayout->addWidget(ocIndicator);
    ui->centerHLayout->addWidget(missionIndicator);

    usbIndicator->setState(StatusIndicator::State::Active);
    hsIndicator->setState(StatusIndicator::State::Active);
    //    ocIndicator->setState(StatusIndicator::State::Active);
    sunIndicator->setState(StatusIndicator::State::Inactive);
    missionIndicator->setState(StatusIndicator::State::Active);

    usbIndicator->setValueText("Подключен");
    hsIndicator->setValueText("Активен");
    //    ocIndicator->setValueText("Активна");
    sunIndicator->setValueText("Нет соединения");
    missionIndicator->setValueText("Загружено");
}

void HalsWindow::on_pushButtonSettings_clicked() {}

void HalsWindow::on_pushButtonSaveParameters_clicked() {}

void HalsWindow::on_pushButtonStartStop_clicked() {
    if (ui->pushButtonStartStop->isChecked()) {
        ui->pushButtonStartStop->setText("  Остановить");
        ui->pushButtonStartStop->setIcon(QIcon(":/Icons/stop-button.png"));
    } else {
        ui->pushButtonStartStop->setText("  Начать эксперимент");
        ui->pushButtonStartStop->setIcon(QIcon(":/Icons/play.png"));
    }
}
