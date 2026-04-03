#include "HalsWindow.h"

#include <BaseTools/IniFileLoader.h>

#include <QMessageBox>
#include <QPixmap>
#include <QStyle>
#include <QTouchEvent>

#include "ui_HalsWindow.h"
#include "version.h"

HalsWindow::HalsWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::HalsWindow), m_touchStartPos(0) {
    setupProject();
    setupGui();
    initObjects();
}

HalsWindow::~HalsWindow() {
    //    qDebug() << "\n\n-------------window destructor";
    m_updatingTimer->stop();
    delete m_facade;
    //    qDebug() << "-------------facade deleted";
    delete ui;
    //    qDebug() << "-------------ui deleted";
}

bool HalsWindow::eventFilter(QObject* watched, QEvent* event) {
    if (watched == ui->stackedWidget && event->type() == QEvent::TouchBegin) {
        QTouchEvent* touchEvent = static_cast<QTouchEvent*>(event);
        if (touchEvent->points().size() == 1) {
            m_touchStartPos = touchEvent->points().first().position().x();
            return true;
        }
    } else if (watched == ui->stackedWidget &&
               event->type() == QEvent::TouchEnd) {
        QTouchEvent* touchEvent = static_cast<QTouchEvent*>(event);
        if (touchEvent->points().size() == 1) {
            qreal endX = touchEvent->points().first().position().x();
            if (endX - m_touchStartPos > 100) {  // сдвиг вправо более 100 px
                if (ui->stackedWidget->currentWidget() == ui->settingsPage) {
                    makePageSwitch(ui->settingsPage, ui->mainPage);
                } else if (ui->stackedWidget->currentWidget() == ui->mainPage) {
                    makePageSwitch(ui->mainPage, ui->settingsPage);
                }
                return true;
            }
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

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

    ui->pushButtonUpdateConfiguration->setIcon(QIcon(":/Icons/refresh.png"));
    ui->pushButtonUpdateConfiguration->setIconSize(QSize(24, 24));

    ui->pushButtonSettings->setIcon(QIcon(":/Icons/setting.png"));
    ui->pushButtonSettings->setIconSize(QSize(24, 24));

    ui->pushButtonStartStop->setIcon(QIcon(":/Icons/play.png"));
    ui->pushButtonStartStop->setIconSize(QSize(24, 24));

    ui->pushButtonToMainPage->setIcon(QIcon(":/Icons/home.png"));
    ui->pushButtonToMainPage->setIconSize(QSize(24, 24));
    ui->pushButtonToMainPage_2->setIcon(QIcon(":/Icons/home.png"));
    ui->pushButtonToMainPage_2->setIconSize(QSize(24, 24));

    ui->pushButtonMakeSnapshot->setIcon(QIcon(":/Icons/save.png"));
    ui->pushButtonMakeSnapshot->setIconSize(QSize(24, 24));

    ui->pushButtonChoosePreset->setIcon(QIcon(":/Icons/control.png"));
    ui->pushButtonChoosePreset->setIconSize(QSize(24, 24));

    QList<QPushButton*> buttons = findChildren<QPushButton*>();
    for (QPushButton* btn : buttons) {
        btn->style()->polish(btn);
    }

    addStatusIndicators();

    ui->stackedWidget->installEventFilter(this);
    connect(ui->pushButtonSettings, &QPushButton::clicked, this,
            [this]() { makePageSwitch(ui->mainPage, ui->settingsPage); });
    connect(ui->pushButtonToMainPage, &QPushButton::clicked, this,
            [this]() { makePageSwitch(ui->settingsPage, ui->mainPage); });
    connect(ui->pushButtonToMainPage_2, &QPushButton::clicked, this,
            [this]() { makePageSwitch(ui->cameraPage, ui->mainPage); });

    if (auto* overlay = qobject_cast<OverlayLabel*>(ui->labelCameraImage))
        overlay->setOverlayRect(m_hsFovRect);
}

void HalsWindow::initSettingsForms() {
    setupSettingBox(ui->widgetHsParams, "Сенсор ГС      ",
                    {ParameterType::HS_EXPOSURE, "Экспозиция", "мс", 1, 2000,
                     50, 5, 50, 100},
                    {ParameterType::HS_FRAMERATE, "Частота регистрации",
                     "кадров/с", 1, 60, 20, 1, 5, 10});

    setupSettingBox(ui->widgetOcParams, "Обзорная камера      ",
                    {ParameterType::OC_EXPOSURE, "Экспозиция", "мс", 0, 1000,
                     10, 1, 10, 50},
                    {ParameterType::OC_FRAMERATE, "Частота регистрации",
                     "кадров/с", 1, 60, 30, 1, 5, 10});

    setupSettingBox(ui->widgetBrightnessParams, "Сенсор освещенности      ",
                    {ParameterType::LIGHT_EXPOSURE, "Экспозиция", "мс", 1, 1000,
                     100, 1, 10, 100},
                    {ParameterType::LIGHT_FRAMERATE, "Частота регистрации",
                     "кадров/с", 1, 60, 1, 1, 5, 10});

    setupSettingBox(ui->widgetExperimentParams, "Эксперимент      ",
                    {ParameterType::EXP_ALTITUDE, "Высота измерений", "м", 2,
                     1000, 2, 1, 10, 100},
                    {ParameterType::EXP_RECORD_START_ALTITUDE,
                     "Высота начала записи", "м", 2, 1000, 50, 1, 10, 100});
}

void HalsWindow::setupSettingBox(DeviceParametersForm* form, QString deviceName,
                                 ParameterInfo firstParameterInfo,
                                 ParameterInfo secondParameterInfo) {
    form->setDeviceName(deviceName);
    form->addParameter(firstParameterInfo);
    form->addParameter(secondParameterInfo);
    connect(form, &DeviceParametersForm::parameterChanged, m_facade,
            &HalsFacade::onParameterChanged);
}

void HalsWindow::applyStyleSheet() {
    QFile file(":/style.qss");
    if (file.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(file.readAll());
        qApp->setStyleSheet(styleSheet);
    } else {
        qWarning() << "Failed to load style.qss from resources";
    }
}

void HalsWindow::addStatusIndicators() {
    m_usbIndicator = new StatusIndicator(this, ":/Icons/flash-drive");
    m_usbIndicator->setLabelText("Флеш-накопитель");

    m_hsIndicator = new StatusIndicator(this, ":/Icons/focus");
    connect(m_hsIndicator, &StatusIndicator::clicked, this,
            [this]() { makePageSwitch(ui->mainPage, ui->cameraPage); });
    m_hsIndicator->setLabelText("Сенсор ГС");

    m_ocIndicator = new StatusIndicator(this, ":/Icons/camera");
    connect(m_ocIndicator, &StatusIndicator::clicked, this,
            [this]() { makePageSwitch(ui->mainPage, ui->cameraPage); });
    m_ocIndicator->setLabelText("Обзорная камера");

    m_sunIndicator = new StatusIndicator(this, ":/Icons/sun");
    m_sunIndicator->setLabelText("Сенсор освещенности");

    m_missionIndicator = new StatusIndicator(this, ":/Icons/checklist");
    m_missionIndicator->setLabelText("Полётное задание");

    ui->centerHLayout->addWidget(m_usbIndicator);
    ui->centerHLayout->addWidget(m_sunIndicator);
    ui->centerHLayout->addWidget(m_hsIndicator);
    ui->centerHLayout->addWidget(m_ocIndicator);
    ui->centerHLayout->addWidget(m_missionIndicator);

    //    sunIndicator->setState(StatusIndicator::State::Inactive);
    //    missionIndicator->setState(StatusIndicator::State::Active);

    //    sunIndicator->setValueText("Не активен");
    //    missionIndicator->setValueText("Загружено");
}

void HalsWindow::makePageSwitch(QWidget* fromPage, QWidget* toPage) {
    if (!fromPage || !toPage) return;
    ui->stackedWidget->setCurrentWidget(toPage);

    if (toPage == ui->cameraPage) {
        m_facade->setVideoStreamEnabled(true);
    } else {
        m_facade->setVideoStreamEnabled(false);
    }
}

void HalsWindow::on_pushButtonUpdateConfiguration_clicked() {
    m_facade->refreshUsbState();
}

void HalsWindow::on_pushButtonStartStop_clicked() {
    if (ui->pushButtonStartStop->isChecked()) {
        ui->pushButtonStartStop->setText("  Остановить");
        ui->pushButtonStartStop->setIcon(QIcon(":/Icons/stop-button.png"));
        m_facade->startExperiment();
    } else {
        ui->pushButtonStartStop->setText("  Начать эксперимент");
        ui->pushButtonStartStop->setIcon(QIcon(":/Icons/play.png"));
        m_facade->stopExperiment();
    }
}

void HalsWindow::setSatellitesCount(const int& satellitesCount) {
    ui->labelGpsValue->setText(QString::number(satellitesCount));
}

void HalsWindow::updateCpuTemperature(QString temperature) {
    ui->labelTemperatureValue->setText(temperature);
}

void HalsWindow::updateUsbState(bool mounted, qint64 availableBytes,
                                qint64 totalBytes) {
    if (mounted) {
        m_usbIndicator->setState(StatusIndicator::State::Active);
        m_usbIndicator->setValueText("Доступно:\n" +
                                     formatBytes(availableBytes) + "/" +
                                     formatBytes(totalBytes) + " GB");
    } else {
        m_usbIndicator->setState(StatusIndicator::State::Inactive);
        m_usbIndicator->setValueText("Не подключен");
    }
}

void HalsWindow::updateHsState(bool connectionStatus) {
    if (connectionStatus) {
        m_hsIndicator->setState(StatusIndicator::State::Active);
        m_hsIndicator->setValueText("Активен");
    } else {
        m_hsIndicator->setState(StatusIndicator::State::Inactive);
        m_hsIndicator->setValueText("Не подключен");
    }
}

void HalsWindow::updateOcState(bool connectionStatus) {
    if (connectionStatus) {
        m_ocIndicator->setState(StatusIndicator::State::Active);
        m_ocIndicator->setValueText("Активен");
    } else {
        m_ocIndicator->setState(StatusIndicator::State::Inactive);
        m_ocIndicator->setValueText("Не подключен");
    }
}

void HalsWindow::updateOcImageLabel(QImage imageToShow, int maxBrightness) {
    QPixmap pix = QPixmap::fromImage(imageToShow);
    ui->labelCameraImage->setPixmap(pix.scaled(ui->labelCameraImage->size(),
                                               Qt::KeepAspectRatio,
                                               Qt::SmoothTransformation));

    if (!m_hsFovRect.isNull()) {
        double scaleX =
            (double)ui->labelCameraImage->width() / imageToShow.width();
        double scaleY =
            (double)ui->labelCameraImage->height() / imageToShow.height();
        QRect scaledRect(m_hsFovRect.x() * scaleX, m_hsFovRect.y() * scaleY,
                         m_hsFovRect.width() * scaleX,
                         m_hsFovRect.height() * scaleY);
        if (auto* overlay = qobject_cast<OverlayLabel*>(ui->labelCameraImage))
            overlay->setOverlayRect(scaledRect);
    }

    ui->widgetOcChars->setMaxBrightness(maxBrightness);
}

void HalsWindow::updateHsImageLabel(QImage imageToShow, int maxBrightness) {
    QPixmap pix = QPixmap::fromImage(imageToShow);
    ui->labelSpectra->setPixmap(pix.scaled(ui->labelSpectra->size(),
                                           Qt::IgnoreAspectRatio,
                                           Qt::SmoothTransformation));

    ui->widgetHsChars->setMaxBrightness(maxBrightness);
}

void HalsWindow::updateOcChars(double fovMeters, double gsd) {
    ui->widgetOcChars->setFov(fovMeters);
    ui->widgetOcChars->setGsd(gsd);
}

void HalsWindow::updateHsChars(double fovMeters, double gsd) {
    ui->widgetHsChars->setFov(fovMeters);
    ui->widgetHsChars->setGsd(gsd);
}

QString HalsWindow::formatBytes(qint64 bytes) {
    return QString::number(bytes / (1024.0 * 1024.0 * 1024.0), 'f', 1);
}

void HalsWindow::updateTime() {
    QDateTime dt = QDateTime::currentDateTimeUtc().addSecs(3 * 3600);
    ui->labelTimeValue->setText(dt.toString("HH:mm:ss"));
}

void HalsWindow::initObjects() {
    m_facade = new HalsFacade();
    connect(m_facade, &HalsFacade::gpsSatellitesCountUpdated, this,
            &HalsWindow::setSatellitesCount);
    connect(m_facade, &HalsFacade::cpuTemperatureUpdated, this,
            &HalsWindow::updateCpuTemperature);
    connect(m_facade, &HalsFacade::usbStatusChanged, this,
            &HalsWindow::updateUsbState);
    connect(m_facade, &HalsFacade::masterConnectionStatusChanged, this,
            &HalsWindow::updateHsState);
    connect(m_facade, &HalsFacade::slaveConnectionStatusChanged, this,
            &HalsWindow::updateOcState);
    connect(m_facade, &HalsFacade::overviewImageReady, this,
            &HalsWindow::updateOcImageLabel);
    connect(m_facade, &HalsFacade::hsImageReady, this,
            &HalsWindow::updateHsImageLabel);
    connect(m_facade, &HalsFacade::ocCharsWereUpdated, this,
            &HalsWindow::updateOcChars);
    connect(m_facade, &HalsFacade::hsCharsWereUpdated, this,
            &HalsWindow::updateHsChars);
    connect(m_facade, &HalsFacade::altitudeWasUpdated, this, [this](int value) {
        ui->labelAltitudeValue->setText(QString::number(value));
    });
    connect(m_facade, &HalsFacade::parameterValueChanged, this,
            &HalsWindow::onForceParameterChanging);
    initSettingsForms();
    m_facade->setVideoStreamEnabled(false);
    m_facade->initialize();

    QString savingPath = m_settings->value("Pathes/saving").toString();
    m_facade->setSavingPath(savingPath);

    m_updatingTimer = new QTimer(this);
    connect(m_updatingTimer, &QTimer::timeout, this, &HalsWindow::updateTime);
    m_updatingTimer->start(1000);
}

void HalsWindow::on_pushButtonMakeSnapshot_clicked() {
    m_facade->makeSnapshot();
}

void HalsWindow::setSpectrometerFovRect(const QRect& rect) {
    if (m_hsFovRect != rect) {
        m_hsFovRect = rect;
        if (auto* overlay = qobject_cast<OverlayLabel*>(ui->labelCameraImage))
            overlay->setOverlayRect(rect);

        m_settings->setValue("Cameras/hsFovXOffset", rect.x());
        m_settings->setValue("Cameras/hsFovYOffset", rect.y());
        m_settings->setValue("Cameras/hsFovWidth", rect.width());
        m_settings->setValue("Cameras/hsFovHeight", rect.height());
    }
}

void HalsWindow::setupProject() {
    ui->setupUi(this);
    m_title.append(VER_PRODUCTNAME_STR)
        .append(" v_")
        .append(VER_FILEVERSION_STR);
    this->setWindowTitle(m_title);

    m_settings = IniFileLoader::createSettingsObject(VER_PRODUCTNAME_STR);

    int hsFovXOffset = m_settings->value("Cameras/hsFovXOffset").toInt();
    int hsFovYOffset = m_settings->value("Cameras/hsFovYOffset").toInt();
    int hsFovWidth = m_settings->value("Cameras/hsFovWidth").toInt();
    int hsFovHeight = m_settings->value("Cameras/hsFovHeight").toInt();
    m_hsFovRect = QRect(hsFovXOffset, hsFovYOffset, hsFovWidth, hsFovHeight);
}

void HalsWindow::on_pushButtonChoosePreset_clicked() {}

void HalsWindow::onForceParameterChanging(ParameterType type, int value) {
    DeviceParametersForm* form = nullptr;
    switch (type) {
        case ParameterType::HS_EXPOSURE:
        case ParameterType::HS_FRAMERATE:
            form = ui->widgetHsParams;
            break;
        case ParameterType::OC_EXPOSURE:
        case ParameterType::OC_FRAMERATE:
            form = ui->widgetOcParams;
            break;
        case ParameterType::LIGHT_EXPOSURE:
        case ParameterType::LIGHT_FRAMERATE:
            form = ui->widgetBrightnessParams;
            break;
        case ParameterType::EXP_ALTITUDE:
        case ParameterType::EXP_RECORD_START_ALTITUDE:
            form = ui->widgetExperimentParams;
            if (type == ParameterType::EXP_ALTITUDE)
                ui->labelAltitudeValue->setText(QString::number(value));
            break;
        default:
            return;
    }
    if (form) {
        form->setParameterValue(type, value);
    }
}
