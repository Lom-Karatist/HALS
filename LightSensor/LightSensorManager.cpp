#include "LightSensorManager.h"

#include <QDebug>
#include <QDir>

LightSensorManager::LightSensorManager(QObject *parent)
    : QObject(parent), m_timer(new QTimer(this)), m_isActive(false) {
    m_api = std::make_unique<LightSensorApi>();
    m_lightSettings =
        std::make_unique<LightSettings>(this, QDir::currentPath() + "/LS.ini");
    m_saver = std::make_unique<LightSaver>();
    connect(m_api.get(), &LightSensorApi::errorOccurred, this,
            &LightSensorManager::errorOccurred);
}

LightSensorManager::~LightSensorManager() { stop(); }

void LightSensorManager::initialize() {
    if (!m_api->initialize()) {
        emit errorOccurred("Light sensor initialization failed");
        return;
    }

    LightSensorParameters params;
    params.exposureMs = m_lightSettings->integrationTimeMs();
    params.gain = m_lightSettings->gainIndex();
    params.fps = m_lightSettings->frameRateHz();

    setIntegrationTimeMs(params.exposureMs);
    setGainIndex(params.gain);
    setFrameRateHz(params.fps);

    emit settingsChanged(params);
}

void LightSensorManager::start() {
    if (m_isActive) return;
    m_isActive = true;
    // Запускаем таймер с интервалом, соответствующим частоте
    int intervalMs = (m_lightSettings->frameRateHz() > 0)
                         ? (1000 / m_lightSettings->frameRateHz())
                         : 100;
    m_timer->start(intervalMs);
}

void LightSensorManager::stop() {
    m_timer->stop();
    m_isActive = false;
}

void LightSensorManager::setIntegrationTimeMs(int ms) {
    if (m_lightSettings->integrationTimeMs() != ms) {
        m_lightSettings->setIntegrationTimeMs(ms);
        if (m_api) m_api->setIntegrationTimeMs(ms);
    }
}

void LightSensorManager::setGainIndex(int index) {
    if (m_lightSettings->gainIndex() != index) {
        m_lightSettings->setGainIndex(index);
        if (m_api) m_api->setGainByIndex(index);
    }
}

void LightSensorManager::setFrameRateHz(int hz) {
    if (m_lightSettings->frameRateHz() != hz) {
        m_lightSettings->setFrameRateHz(hz);
        if (m_isActive) {
            m_timer->stop();
            int intervalMs = (hz > 0) ? (1000 / hz) : 100;
            m_timer->start(intervalMs);
        }
    }
}

void LightSensorManager::setSavingPath(const QString &path) {
    m_saver->setSavingPath(path);
}

void LightSensorManager::onTimer() {
    LightSensorData data;
    if (m_api->readAllChannels(data)) {
        emit dataReady(data);
        if (m_saver->isEnabled()) {
            m_saver->saveDataAsync(data);
        }
    }
}
