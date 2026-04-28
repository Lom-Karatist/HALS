#include "LightSensorWorker.h"

#include <QDebug>
#include <QThread>
#include <chrono>
#include <thread>

LightSensorWorker::LightSensorWorker(QObject *parent)
    : QObject(parent),
      m_api(std::make_unique<LightSensorApi>()),
      m_isActive(false),
      m_integrationTimeMs(50),
      m_gainIndex(6),
      m_frameRateHz(10),
      m_isCommandsPending(false) {
    connect(m_api.get(), &LightSensorApi::errorOccurred, this,
            &LightSensorWorker::errorOccurred);
}

LightSensorWorker::~LightSensorWorker() { stopWork(); }

void LightSensorWorker::run() {
    if (!m_api->initialize()) {
        emit errorOccurred("Light sensor initialization failed");
        emit connectionStatusChanged(false);
        return;
    }
    emit connectionStatusChanged(true);

    m_api->setIntegrationTimeMs(m_integrationTimeMs);
    m_api->setGainByIndex(m_gainIndex);

    LightSensorParameters params;
    params.exposureMs = m_integrationTimeMs;
    params.gain = m_gainIndex;
    params.fps = m_frameRateHz;
    emit settingsChanged(params);

    qDebug() << "Light sensor start";

    while (m_isActive.load()) {
        if (m_isCommandsPending.load()) {
            applyPendingCommands();
            continue;
        }

        int intervalMs = (m_frameRateHz > 0) ? (1000 / m_frameRateHz) : 100;
        auto startTime = std::chrono::steady_clock::now();

        LightSensorData data;
        if (m_api->readAllChannels(data)) {
            data.sunElevation = 0.0;
            emit dataReady(data);
        }

        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                           std::chrono::steady_clock::now() - startTime)
                           .count();
        if (elapsed < intervalMs) {
            std::this_thread::sleep_for(
                std::chrono::milliseconds(intervalMs - elapsed));
        }
    }
}

void LightSensorWorker::activate() { m_isActive.store(true); }

void LightSensorWorker::stopWork() { m_isActive.store(false); }

void LightSensorWorker::setIntegrationTimeMs(int ms) {
    QMutexLocker locker(&m_cmdMutex);
    m_pendingCommands.push_back(
        {LightCommandType ::SetIntegrationTime, ms, 0.0});
    m_isCommandsPending.store(true);
}

void LightSensorWorker::setGainIndex(int index) {
    QMutexLocker locker(&m_cmdMutex);
    m_pendingCommands.push_back({LightCommandType ::SetGainIndex, index, 0.0});
    m_isCommandsPending.store(true);
}

void LightSensorWorker::setFrameRateHz(int hz) {
    QMutexLocker locker(&m_cmdMutex);
    m_pendingCommands.push_back({LightCommandType ::SetFrameRate, hz, 0.0});
    m_isCommandsPending.store(true);
}

void LightSensorWorker::applyPendingCommands() {
    std::vector<LightCommand> commands;
    {
        QMutexLocker locker(&m_cmdMutex);
        commands.swap(m_pendingCommands);
        m_isCommandsPending.store(false);
    }
    for (const auto &cmd : commands) {
        switch (cmd.type) {
            case LightCommandType::SetIntegrationTime:
                m_integrationTimeMs = cmd.intValue;
                if (m_api) m_api->setIntegrationTimeMs(m_integrationTimeMs);
                break;
            case LightCommandType::SetGainIndex:
                m_gainIndex = cmd.intValue;
                if (m_api) m_api->setGainByIndex(m_gainIndex);
                break;
            case LightCommandType::SetFrameRate:
                m_frameRateHz = cmd.intValue;
                break;
            default:
                break;
        }
    }

    LightSensorParameters params;
    params.exposureMs = m_integrationTimeMs;
    params.gain = m_gainIndex;
    params.fps = m_frameRateHz;
    emit settingsChanged(params);
}

int LightSensorWorker::frameRateHz() const { return m_frameRateHz; }

int LightSensorWorker::gainIndex() const { return m_gainIndex; }

int LightSensorWorker::integrationTimeMs() const { return m_integrationTimeMs; }
