#include "HalsFacade.h"

#include <QDir>

HalsFacade::HalsFacade(QObject *parent) : QObject(parent) {
    m_satellitesCount = -1;
}

HalsFacade::~HalsFacade() {
    qDebug() << "----------------ending facade";

    m_loggerThread->quit();
    m_loggerThread->wait();

    qDebug() << "----------------logger finish";

    m_tempControllerThread->quit();
    m_tempControllerThread->wait();

    qDebug() << "----------------CPU TC finish";

    if (m_gpsDevice) m_gpsDevice->stop();

    qDebug() << "----------------GPS finish";

    stopCapture();

    qDebug() << "----------------Basler finish";
}

void HalsFacade::initialize() {
    startLogger();
    startTempController();
    startUsbChecker();
    startCameras();
    startGps();
}

void HalsFacade::refreshUsbState() {
    if (m_usbChecker) m_usbChecker->check();
}

void HalsFacade::startLogger() {
    m_loggerThread = new QThread(this);
    m_logger = std::make_unique<Logger>();
    m_logger->moveToThread(m_loggerThread);

    connect(m_loggerThread, &QThread::finished, m_logger.get(),
            &QObject::deleteLater);
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
            &HalsFacade::usbStatusChanged);
    m_usbChecker->check();
}

bool HalsFacade::startCameras() {
    m_cameraManager = std::make_unique<CameraManager>(this);
    if (m_cameraManager) {
        connect(m_cameraManager.get(),
                &CameraManager::slaveConnectionStatusChanged, this,
                &HalsFacade::slaveConnectionStatusChanged);
        connect(m_cameraManager.get(),
                &CameraManager::masterConnectionStatusChanged, this,
                &HalsFacade::masterConnectionStatusChanged);
        connect(m_cameraManager.get(), &CameraManager::slaveImageReady, this,
                &HalsFacade::overviewImageReady);
        connect(m_cameraManager.get(), &CameraManager::masterRawData, this,
                &HalsFacade::updateHsData);
        connect(m_cameraManager.get(), &CameraManager::errorOccurred, this,
                &HalsFacade::cameraError, Qt::QueuedConnection);
        m_cameraManager->initCameras();
        return true;
    } else {
        logMessage(Logger::Critical, "Hyperspectrometer sensors init error");
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

void HalsFacade::startCapture() {
    if (m_cameraManager) {
        m_cameraManager->start();
    }
}

void HalsFacade::pauseCapture() {
    if (m_cameraManager) {
        m_cameraManager->pause();
    }
}

void HalsFacade::stopCapture() {
    if (m_cameraManager) {
        m_cameraManager->stop();
    }
}

void HalsFacade::setSaveFormat(int format) {
    // saving format is need to be set in camera manager
}

bool HalsFacade::isHSCameraReady() const {
    return m_cameraManager && m_cameraManager->isReady();
}

bool HalsFacade::isOCCameraReady() const {
    return m_cameraManager && m_cameraManager->isReady();
}

bool HalsFacade::isLightSensorReady() const {
    // Пока заглушка
    return true;
}

bool HalsFacade::isMissionLoaded() const {
    // Заглушка
    return true;
}

void HalsFacade::onGpsDataUpdated(const GpsData &gpsData) {
    if (m_satellitesCount != gpsData.satellites) {
        m_satellitesCount = gpsData.satellites;
        emit gpsSatellitesCountUpdated(m_satellitesCount);
    }
}

void HalsFacade::updateHsData(const QByteArray &data, int w, int h) {}
