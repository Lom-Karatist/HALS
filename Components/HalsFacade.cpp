#include "HalsFacade.h"

#include <QDir>

HalsFacade::HalsFacade(QObject *parent) : QObject(parent) {
    initialize();
    m_satellitesCount = -1;
}

HalsFacade::~HalsFacade() {
    if (m_gpsDevice) m_gpsDevice->stop();
    stopCapture();
}

void HalsFacade::initialize() {
    startCameras();
    startGps();
}

bool HalsFacade::startCameras() {
    m_cameraManager = std::make_unique<CameraManager>(this);
    if (m_cameraManager) {
        connect(m_cameraManager.get(), &CameraManager::slaveImageReady, this,
                &HalsFacade::overviewImageReady);
        connect(m_cameraManager.get(), &CameraManager::errorOccurred, this,
                &HalsFacade::cameraError, Qt::QueuedConnection);
        return true;
    } else
        return false;
}

bool HalsFacade::startGps() {
    m_gpsDevice = std::make_unique<GPSDevice>(EmitMode::BothValid, this);
    if (m_gpsDevice) {
        connect(m_gpsDevice.get(), &GPSDevice::gpsDataUpdated, this,
                &HalsFacade::onGpsDataUpdated, Qt::QueuedConnection);
        m_gpsDevice->start();
        return true;
    } else
        return false;
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

bool HalsFacade::isFlashMounted() const {
    // Можно проверить наличие монтирования USB, но пока true
    return true;
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
