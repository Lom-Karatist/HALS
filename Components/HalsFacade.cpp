#include "HalsFacade.h"

#include <QDir>

HalsFacade::HalsFacade(QObject *parent) : QObject(parent) {
    m_satellitesCount = -1;
}

HalsFacade::~HalsFacade() {
    //    qDebug() << "----------------ending facade";

    m_loggerThread->quit();
    m_loggerThread->wait();

    //    qDebug() << "----------------logger finish";
    if (m_tempController) {
        QMetaObject::invokeMethod(m_tempController.get(), "stopTimer",
                                  Qt::BlockingQueuedConnection);
    }
    m_tempControllerThread->quit();
    m_tempControllerThread->wait();

    //    qDebug() << "----------------CPU TC finish";

    if (m_gpsDevice) m_gpsDevice->stop();

    //    qDebug() << "----------------GPS finish";

    stopBaslerCameras();

    //    qDebug() << "----------------Basler finish";
}

void HalsFacade::initialize() {
    startLogger();
    startTempController();
    startUsbChecker();
    initCameras();
    initFlightTaskModule();
    startGps();
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
            &HalsFacade::usbStatusChanged);
    m_usbChecker->check();
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

void HalsFacade::initFlightTaskModule() {
    m_flightTaskModule = std::make_unique<FlightTaskModule>(
        this, m_cameraManager->ocParams().width,
        m_cameraManager->hsParams().height);
    connect(m_flightTaskModule.get(), &FlightTaskModule::ocCharsWereUpdated,
            this, &HalsFacade::ocCharsWereUpdated, Qt::QueuedConnection);
    connect(m_flightTaskModule.get(), &FlightTaskModule::hsCharsWereUpdated,
            this, &HalsFacade::hsCharsWereUpdated, Qt::QueuedConnection);
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

void HalsFacade::setSaveFormat(int format) {
    // saving format is need to be set in camera manager
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
