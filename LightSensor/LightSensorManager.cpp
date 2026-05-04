#include "LightSensorManager.h"

#include <QDebug>
#include <QDir>
#include <QThreadPool>

LightSensorManager::LightSensorManager(QObject *parent)
    : QObject(parent), m_currentSunElevation(0.0) {
    m_lightSettings =
        std::make_unique<LightSettings>(this, QDir::currentPath() + "/LS.ini");
    m_saver = std::make_unique<LightSaver>();
}

LightSensorManager::~LightSensorManager() {
    stopAs7341Stream();
#ifdef Q_OS_LINUX
    if (m_udpThread) {
        if (m_udpReader) {
            m_udpReader->stop();
            m_udpReader->deleteLater();
        }
        m_udpThread->quit();
        m_udpThread->wait();
        delete m_udpThread;
    }
#endif
}

void LightSensorManager::startAs7341Stream(int expoMs, int gainIndex,
                                           int framerateHz) {
#ifdef Q_OS_LINUX
    // Путь к папке со скриптом – измените под свою конфигурацию
    QString scriptDir = "/home/hals/Desktop/python";
    QString scriptPath = scriptDir + "/as7341_stream.py";
    QString python = "python3";

    m_lsProcess = new QProcess(this);
    m_lsProcess->setWorkingDirectory(scriptDir);

    QStringList args;
    args << scriptPath << "--integration" << QString::number(expoMs) << "--gain"
         << QString::number(gainIndex) << "--freq"
         << QString::number(framerateHz) << "--port"
         << "12345";

    m_lsProcess->start(python, args);
    if (!m_lsProcess->waitForStarted(3000)) {
        qDebug() << "Failed to start as7341_stream.py:"
                 << m_lsProcess->errorString();
        emit connectionStatusChanged(false);
        return;
    }
    qDebug() << "AS7341 Python script started";
#endif
}

void LightSensorManager::stopAs7341Stream() {
#ifdef Q_OS_LINUX
    if (m_lsProcess) {
        m_lsProcess->terminate();
        if (!m_lsProcess->waitForFinished(2000)) m_lsProcess->kill();
        delete m_lsProcess;
        m_lsProcess = nullptr;
    }
#endif
}

void LightSensorManager::initialize() {
    qDebug() << "Initializing LightSensorManager";

    LightSensorParameters params;
    params.exposureMs = m_lightSettings->integrationTimeMs();
    params.gain = m_lightSettings->gainIndex();
    params.fps = m_lightSettings->frameRateHz();

#ifdef Q_OS_LINUX
    // 1. Запускаем Python-скрипт
    startAs7341Stream(params.exposureMs, params.gain, params.fps);

    // 2. Запускаем UDP-читатель в отдельном потоке
    m_udpThread = new QThread(this);
    m_udpReader = new UdpLightSensorReader(12345);
    m_udpReader->moveToThread(m_udpThread);

    connect(m_udpThread, &QThread::started, m_udpReader,
            &UdpLightSensorReader::start);
    connect(m_udpReader, &UdpLightSensorReader::dataReceived, this,
            &LightSensorManager::onDataReady);
    connect(m_udpThread, &QThread::finished, m_udpReader,
            &QObject::deleteLater);

    m_udpThread->start();
    emit connectionStatusChanged(true);
#else
    qDebug()
        << "LightSensorManager: running in windows mode (no Python script)";
    emit connectionStatusChanged(false);
#endif
}

void LightSensorManager::setIntegrationTimeMs(int ms) {
    if (m_lightSettings->integrationTimeMs() != ms) {
        m_lightSettings->setIntegrationTimeMs(ms);
    }
}

void LightSensorManager::setGainIndex(int index) {
    if (m_lightSettings->gainIndex() != index) {
        m_lightSettings->setGainIndex(index);
    }
}

void LightSensorManager::setFrameRateHz(int hz) {
    if (m_lightSettings->frameRateHz() != hz) {
        m_lightSettings->setFrameRateHz(hz);
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
