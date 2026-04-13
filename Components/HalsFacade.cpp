#include "HalsFacade.h"

#include <QDir>

HalsFacade::HalsFacade(QObject *parent) : QObject(parent) {
    m_satellitesCount = -1;
}

HalsFacade::~HalsFacade() {
    //    qDebug() << "----------------ending facade";

    if (m_loggerThread) {
        m_loggerThread->quit();
        m_loggerThread->wait();
    }

    //    qDebug() << "----------------logger finish";
    if (m_tempController) {
        QMetaObject::invokeMethod(m_tempController.get(), "stopTimer",
                                  Qt::BlockingQueuedConnection);
        m_tempControllerThread->quit();
        m_tempControllerThread->wait();
    }

    if (m_saverThread) {
        m_saverThread->quit();
        m_saverThread->wait();
    }

    //    qDebug() << "----------------CPU TC finish";

    if (m_gpsDevice) m_gpsDevice->stop();

    //    qDebug() << "----------------GPS finish";

    stopBaslerCameras();

    //    qDebug() << "----------------Basler finish";
}

void HalsFacade::initialize() {
    startLogger();
    startTempController();
    initDataSaver();
    initCameras();
    initFlightTaskModule();
    startUsbChecker();
    startGps();
    initLightSensor();
    startMemoryMonitor();
    initExperimentController();
}

void HalsFacade::refreshUsbState() {
    if (m_usbChecker) m_usbChecker->check();
}

void HalsFacade::startLogger() {
    m_loggerThread = new QThread(this);
    m_logger = std::make_unique<Logger>();
    m_logger->moveToThread(m_loggerThread);

    connect(this, &HalsFacade::logMessage, m_logger.get(), &Logger::log,
            Qt::QueuedConnection);
    m_loggerThread->start();
    logMessage(Logger::Info, "HALS facade started");
}

void HalsFacade::startTempController() {
    m_tempControllerThread = new QThread(this);
    m_tempController = std::make_unique<CpuTemperatureController>();
    m_tempController->moveToThread(m_tempControllerThread);
    connect(m_tempController.get(),
            &CpuTemperatureController::cpuTemperatureUpdated, this,
            &HalsFacade::cpuTemperatureUpdated);
    m_tempControllerThread->start();
}

void HalsFacade::startUsbChecker() {
    m_usbChecker = std::make_unique<UsbChecker>(this);
    connect(m_usbChecker.get(), &UsbChecker::usbStatusChanged, this,
            &HalsFacade::onUsbStatusChanged);
    connect(m_usbChecker.get(), &UsbChecker::usbSpaceUpdated, m_dataSaver.get(),
            &DataSaver::onUsbSpaceUpdated);
    m_usbChecker->check();
    m_usbChecker->startMonitoring();
}

void HalsFacade::initDataSaver() {
    m_saverThread = new QThread(this);
    m_dataSaver = std::make_unique<DataSaver>();
    m_dataSaver->moveToThread(m_saverThread);
    m_saverThread->start();
}

bool HalsFacade::initCameras() {
    m_cameraManager = std::make_unique<CameraManager>(this);
    if (m_cameraManager) {
        connect(m_cameraManager.get(),
                &CameraManager::slaveConnectionStatusChanged, this,
                &HalsFacade::slaveConnectionStatusChanged);
        connect(m_cameraManager.get(),
                &CameraManager::masterConnectionStatusChanged, this,
                &HalsFacade::masterConnectionStatusChanged);
        connect(m_cameraManager.get(), &CameraManager::ready, this,
                &HalsFacade::startBaslerCameras);
        connect(m_cameraManager.get(), &CameraManager::errorOccurred, this,
                &HalsFacade::componentError, Qt::QueuedConnection);
        m_cameraManager->initCameras();

        connect(m_cameraManager.get(), &CameraManager::forceParameterChanging,
                this, &HalsFacade::onCameraForceParameterChanging);

        emit parameterValueChanged(ParameterType::HS_EXPOSURE,
                                   m_cameraManager->hsParams().exposureTime);
        emit parameterValueChanged(
            ParameterType::HS_FRAMERATE,
            m_cameraManager->hsParams().acquisitionFrameRate);
        emit parameterValueChanged(ParameterType::OC_EXPOSURE,
                                   m_cameraManager->ocParams().exposureTime);
        emit parameterValueChanged(
            ParameterType::OC_FRAMERATE,
            m_cameraManager->ocParams().acquisitionFrameRate);
        return true;
    } else {
        emit logMessage(Logger::Critical,
                        "Hyperspectrometer sensors init error");
        return false;
    }
}

bool HalsFacade::startGps() {
    m_gpsDevice = std::make_unique<GPSDevice>(EmitMode::BothValid, this);
    if (m_gpsDevice) {
        connect(m_gpsDevice.get(), &GPSDevice::gpsDataUpdated, this,
                &HalsFacade::onGpsDataUpdated, Qt::QueuedConnection);
        m_gpsDevice->start();
        logMessage(Logger::Info, "GPS initted");
        return true;
    } else {
        logMessage(Logger::Critical, "GPS init error");
        return false;
    }
}

void HalsFacade::initExperimentController() {
    m_experimentController = std::make_unique<ExperimentController>(this);
    m_experimentController->setCameraManager(m_cameraManager.get());
    m_experimentController->setFlightTaskModule(m_flightTaskModule.get());
    m_experimentController->setDataSaver(m_dataSaver.get());
    m_experimentController->setLightSensor(m_lightSensorManager.get());
    m_experimentController->setGpsDevice(m_gpsDevice.get());

    connect(m_experimentController.get(),
            &ExperimentController::sunElevationUpdated,
            m_lightSensorManager.get(),
            &LightSensorManager::updateSunElevation);

    connect(m_experimentController.get(),
            &ExperimentController::experimentStarted, this, [this]() {
                if (m_usbChecker) m_usbChecker->startMonitoring(5000);
            });
    connect(m_experimentController.get(),
            &ExperimentController::experimentStopped, this, [this]() {
                if (m_usbChecker) m_usbChecker->stopMonitoring();
            });
    connect(m_experimentController.get(),
            &ExperimentController::experimentStarted, this, [this]() {
                if (m_memoryMonitor) m_memoryMonitor->startMonitoring(5000);
            });
    connect(m_experimentController.get(),
            &ExperimentController::experimentStopped, this, [this]() {
                if (m_memoryMonitor) m_memoryMonitor->stopMonitoring();
            });
}

void HalsFacade::initFlightTaskModule() {
    m_flightTaskModule = std::make_unique<FlightTaskModule>(
        this, m_cameraManager->ocParams().width,
        m_cameraManager->hsParams().height);
    connect(m_flightTaskModule.get(), &FlightTaskModule::ocCharsWereUpdated,
            this, &HalsFacade::ocCharsWereUpdated, Qt::QueuedConnection);
    connect(m_flightTaskModule.get(), &FlightTaskModule::hsCharsWereUpdated,
            this, &HalsFacade::hsCharsWereUpdated, Qt::QueuedConnection);
    connect(m_flightTaskModule.get(), &FlightTaskModule::altitudeWasUpdated,
            this, &HalsFacade::altitudeWasUpdated, Qt::QueuedConnection);
    connect(m_flightTaskModule.get(), &FlightTaskModule::missionLoaded, this,
            &HalsFacade::flightTaskLoaderStatusChanged, Qt::QueuedConnection);

    emit parameterValueChanged(ParameterType::EXP_ALTITUDE,
                               m_flightTaskModule->flightAltitude());
    m_flightTaskModule->updateChars();
}

void HalsFacade::initLightSensor() {
    m_lightSensorManager = std::make_unique<LightSensorManager>(this);
    qRegisterMetaType<LightSensorData>();
    qRegisterMetaType<LightSensorParameters>();
    connect(m_lightSensorManager.get(), &LightSensorManager::dataReady, this,
            &HalsFacade::onLightDataReady);
    connect(m_lightSensorManager.get(), &LightSensorManager::settingsChanged,
            this, &HalsFacade::onLightForceParameterChanging);
    connect(m_lightSensorManager.get(), &LightSensorManager::errorOccurred,
            this, &HalsFacade::componentError);
    connect(m_lightSensorManager.get(),
            &LightSensorManager::connectionStatusChanged, this,
            &HalsFacade::lightSensorConnectionStatusChanged);
    m_lightSensorManager->initialize();
}

void HalsFacade::startMemoryMonitor() {
    m_memoryMonitor = std::make_unique<MemoryMonitor>(this);
    connect(m_memoryMonitor.get(), &MemoryMonitor::memoryUsageUpdated,
            m_dataSaver.get(), &DataSaver::onMemoryUsageUpdated);
}

void HalsFacade::stopBaslerCameras() {
    if (m_cameraManager) {
        m_cameraManager->stop();
    }
}

void HalsFacade::setVideoStreamEnabled(bool enabled) {
    if (m_cameraManager) {
        m_cameraManager->setIsImageNeeded(enabled);
    }
}

void HalsFacade::setFlightAltitude(int altitude) {
    if (m_flightTaskModule) {
        m_flightTaskModule->setAltitude(altitude);
    }
}

void HalsFacade::setSavingPath(QString savingPath) {
    m_dataSaver->setSavingPath(savingPath);
    m_cameraManager->setSavingPath(savingPath);
    m_lightSensorManager->setSavingPath(savingPath);
    m_gpsDevice->setSavingPath(savingPath);
}

void HalsFacade::setSaveFormat(int format) {
    // saving format is need to be set in camera manager
}

void HalsFacade::makeSnapshot() { m_cameraManager->makeSingleShootNeeded(); }

void HalsFacade::startExperiment() {
    m_experimentController->forceStartExperiment();
}

void HalsFacade::stopExperiment() {
    m_experimentController->forceStopExperiment();
}

void HalsFacade::loadPreset(const QString &presetName) {
    if (m_experimentController->experimentActive()) {
        stopExperiment();
    }
    setVideoStreamEnabled(false);
}

void HalsFacade::onParameterChanged(ParameterType type, int newValue) {
    switch (type) {
        case ParameterType::HS_EXPOSURE:
            if (m_cameraManager)
                m_cameraManager->onSettingsChanged(
                    true, BaslerConstants::SettingTypes::Exposure, newValue);
            break;
        case ParameterType::HS_FRAMERATE:
            if (m_cameraManager)
                m_cameraManager->onSettingsChanged(
                    true, BaslerConstants::SettingTypes::AcquisitionFramerate,
                    newValue);
            break;
        case ParameterType::OC_EXPOSURE:
            if (m_cameraManager)
                m_cameraManager->onSettingsChanged(
                    false, BaslerConstants::SettingTypes::Exposure, newValue);
            break;
        case ParameterType::OC_FRAMERATE:
            if (m_cameraManager)
                m_cameraManager->onSettingsChanged(
                    false, BaslerConstants::SettingTypes::AcquisitionFramerate,
                    newValue);
            break;
        case ParameterType::LIGHT_EXPOSURE:
            if (m_lightSensorManager)
                m_lightSensorManager->setIntegrationTimeMs(newValue);
            break;
        case ParameterType::LIGHT_FRAMERATE:
            if (m_lightSensorManager)
                m_lightSensorManager->setFrameRateHz(newValue);
            break;
        case ParameterType::EXP_ALTITUDE:
            setFlightAltitude(newValue);
            break;
        case ParameterType::EXP_RECORD_START_ALTITUDE:
            // сохранить в настройках эксперимента
            break;
        default:
            break;
    }
}

void HalsFacade::onGpsDataUpdated(const GpsData &gpsData) {
    if (m_satellitesCount != gpsData.satellites) {
        m_satellitesCount = gpsData.satellites;
        emit gpsSatellitesCountUpdated(m_satellitesCount);
    }
    if (m_experimentController) {
        m_experimentController->updateGpsData(gpsData);
    }
}

void HalsFacade::startBaslerCameras() {
    connect(m_cameraManager.get(), &CameraManager::slaveImageReady, this,
            &HalsFacade::overviewImageReady);
    connect(m_cameraManager.get(), &CameraManager::masterImageReady, this,
            &HalsFacade::hsImageReady);
    connect(m_cameraManager.get(), &CameraManager::masterRawData, this,
            &HalsFacade::updateHsData);
    m_cameraManager->start();
}

void HalsFacade::updateHsData(const QByteArray &data, int w, int h,
                              int pixelFormat) {}

void HalsFacade::onCameraForceParameterChanging(
    bool isMaster, BaslerConstants::SettingTypes settingType, QVariant value) {
    ParameterType type;
    if (isMaster) {
        if (settingType == BaslerConstants::SettingTypes::Exposure)
            type = ParameterType::HS_EXPOSURE;
        else if (settingType ==
                 BaslerConstants::SettingTypes::AcquisitionFramerate)
            type = ParameterType::HS_FRAMERATE;
        else
            return;
    } else {
        if (settingType == BaslerConstants::SettingTypes::Exposure)
            type = ParameterType::OC_EXPOSURE;
        else if (settingType ==
                 BaslerConstants::SettingTypes::AcquisitionFramerate)
            type = ParameterType::OC_FRAMERATE;
        else
            return;
    }
    emit parameterValueChanged(type, value.toInt());
}

void HalsFacade::onLightForceParameterChanging(
    const LightSensorParameters &params) {
    emit parameterValueChanged(ParameterType::LIGHT_EXPOSURE,
                               params.exposureMs);
    emit parameterValueChanged(ParameterType::LIGHT_FRAMERATE, params.fps);
}

void HalsFacade::onLightDataReady(const LightSensorData &data) {
    // data ready slot
}

void HalsFacade::onUsbStatusChanged(bool mounted, qint64 availableBytes,
                                    qint64 totalBytes) {
    emit usbStatusChanged(mounted, availableBytes, totalBytes);

    if (!mounted)
        emit flightTaskLoaderStatusChanged(false);
    else {
        if (!m_flightTaskModule->missionValid()) {
            if (m_flightTaskModule->loadMission(m_usbChecker->lastPath())) {
                emit parameterValueChanged(
                    ParameterType::EXP_ALTITUDE,
                    m_flightTaskModule->flightAltitude());
                emit parameterValueChanged(
                    ParameterType::EXP_RECORD_START_ALTITUDE,
                    m_flightTaskModule->shootingAltitude());
            }
        }
    }
}
