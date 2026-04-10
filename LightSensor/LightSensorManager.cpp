#include "LightSensorManager.h"

#include <QDebug>
#include <QDir>
#include <QThreadPool>

LightSensorManager::LightSensorManager(QObject *parent)
    : QObject(parent), m_worker(nullptr), m_currentSunElevation(0.0) {
    m_lightSettings =
        std::make_unique<LightSettings>(this, QDir::currentPath() + "/LS.ini");
    m_saver = std::make_unique<LightSaver>();
}

LightSensorManager::~LightSensorManager() {
    if (m_worker) {
        m_worker->stopWork();
        QThreadPool::globalInstance()->waitForDone(2000);
    }
    if (m_worker) {
        delete m_worker;
        m_worker = nullptr;
    }
}

void LightSensorManager::initialize() {
    m_worker = new LightSensorWorker();
    m_worker->setAutoDelete(false);

    connect(m_worker, &LightSensorWorker::dataReady, this,
            &LightSensorManager::onDataReady);
    connect(m_worker, &LightSensorWorker::errorOccurred, this,
            &LightSensorManager::errorOccurred);
    connect(m_worker, &LightSensorWorker::connectionStatusChanged, this,
            &LightSensorManager::connectionStatusChanged);
    connect(m_worker, &LightSensorWorker::settingsChanged, this,
            &LightSensorManager::settingsChanged);

    LightSensorParameters params;
    params.exposureMs = m_lightSettings->integrationTimeMs();
    params.gain = m_lightSettings->gainIndex();
    params.fps = m_lightSettings->frameRateHz();

    setIntegrationTimeMs(params.exposureMs);
    setGainIndex(params.gain);
    setFrameRateHz(params.fps);

    emit connectionStatusChanged(true);
    emit settingsChanged(params);

    m_worker->activate();
    QThreadPool::globalInstance()->start(m_worker);
}

void LightSensorManager::setIntegrationTimeMs(int ms) {
    if (m_lightSettings->integrationTimeMs() != ms) {
        m_lightSettings->setIntegrationTimeMs(ms);
    }
    if (m_worker) {
        if (m_worker->integrationTimeMs() != ms)
            m_worker->setIntegrationTimeMs(ms);
    }
}

void LightSensorManager::setGainIndex(int index) {
    if (m_lightSettings->gainIndex() != index) {
        m_lightSettings->setGainIndex(index);
    }
    if (m_worker) {
        if (m_worker->gainIndex() != index) m_worker->setGainIndex(index);
    }
}

void LightSensorManager::setFrameRateHz(int hz) {
    if (m_lightSettings->frameRateHz() != hz) {
        m_lightSettings->setFrameRateHz(hz);
    }
    if (m_worker) {
        if (m_worker->frameRateHz() != hz) m_worker->setFrameRateHz(hz);
    }
}

void LightSensorManager::setSavingPath(const QString &path) {
    m_saver->setSavingPath(path);
}

void LightSensorManager::setRecordingEnabled(bool enabled) {
    m_saver->setEnabled(enabled);
}

void LightSensorManager::updateSunElevation(double elevation) {
    m_currentSunElevation.store(elevation);
}

void LightSensorManager::onDataReady(LightSensorData data) {
    data.sunElevation = m_currentSunElevation.load();
    emit dataReady(data);
    if (m_saver->isEnabled()) {
        m_saver->saveDataAsync(data);
    }
}
