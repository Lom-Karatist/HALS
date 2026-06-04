#include "LightSensorManager.h"

#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <QThreadPool>

LightSensorManager::LightSensorManager(QObject *parent)
    : QObject(parent), m_currentSunElevation(0.0) {
    m_lightSettings = std::make_unique<LightSettings>(
        this,
        QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) +
            "/LS.ini");
    m_saver = std::make_unique<LightSaver>();
    m_recordingMode = IndependentMode;
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
    int atime, astep;
    msToAtimeAstep(expoMs, atime, astep);

    QString scriptDir = "/home/hals/Desktop/python";
    QString scriptPath = scriptDir + "/as7341_stream.py";
    QString python = "python3";

    static const double gainMap[] = {0.5, 1,  2,   4,   8,  16,
                                     32,  64, 128, 256, 512};
    double gainValue =
        (gainIndex >= 0 && gainIndex <= 10) ? gainMap[gainIndex] : 32.0;

    m_lsProcess = new QProcess(this);
    m_lsProcess->setWorkingDirectory(scriptDir);

    connect(m_lsProcess, &QProcess::readyReadStandardOutput, [=]() {
        qDebug() << "Python stdout:" << m_lsProcess->readAllStandardOutput();
    });
    connect(m_lsProcess, &QProcess::readyReadStandardError, [=]() {
        qDebug() << "Python stderr:" << m_lsProcess->readAllStandardError();
    });

    QStringList args;
    args << scriptPath << "--atime" << QString::number(atime) << "--astep"
         << QString::number(astep) << "--gain" << QString::number(gainValue)
         << "--freq" << QString::number(framerateHz) << "--port"
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
    if (m_commandWriter != nullptr) delete m_commandWriter;
#endif
}

void LightSensorManager::initialize() {
    qDebug() << "Initializing LightSensorManager";

    LightSensorParameters params;
    params.exposureMs = m_lightSettings->integrationTimeMs();
    params.gain = m_lightSettings->gainIndex();
    params.fps = m_lightSettings->frameRateHz();
    emit settingsChanged(params);

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

    m_commandWriter = new UdpLightSensorWriter("127.0.0.1", 12346, this);

#else
    qDebug()
        << "LightSensorManager: running in windows mode (no Python script)";
    emit connectionStatusChanged(false);
#endif
}

void LightSensorManager::setIntegrationTimeMs(int ms) {
    if (m_lightSettings->integrationTimeMs() != ms) {
        m_lightSettings->setIntegrationTimeMs(ms);
#ifdef Q_OS_LINUX
        if (m_commandWriter) {
            int atime, astep;
            msToAtimeAstep(ms, atime, astep);
            m_commandWriter->sendIntegrationTime(atime, astep);
        }
#endif
    }
}

void LightSensorManager::setGainIndex(int index) {
    if (m_lightSettings->gainIndex() != index) {
        m_lightSettings->setGainIndex(index);
#ifdef Q_OS_LINUX
        if (m_commandWriter) {
            double gainValue = ls_gain_transorms::gainIndexToValue(index);
            m_commandWriter->sendGain(gainValue);
        }
#endif
    }
}

void LightSensorManager::setFrameRateHz(int hz) {
    if (m_lightSettings->frameRateHz() != hz) {
        m_lightSettings->setFrameRateHz(hz);
#ifdef Q_OS_LINUX
        if (m_commandWriter) {
            m_commandWriter->sendFrameRate(hz);
        }
#endif
    }
}

void LightSensorManager::setSavingPath(const QString &path) {
    m_saver->setSavingPath(path);
}

void LightSensorManager::setRecordingEnabled(bool enabled,
                                             bool isIndependentSavingNeeded) {
    m_saver->setEnabled(enabled);
    if (isIndependentSavingNeeded) {
        m_recordingMode = IndependentMode;
    } else {
        m_recordingMode = BatchMode;
    }
}

void LightSensorManager::updateSunElevation(double elevation) {
    m_currentSunElevation.store(elevation);
}

void LightSensorManager::onDataReady(LightSensorData data) {
    data.sunElevation = m_currentSunElevation.load();
    if (m_saver->isEnabled()) {
        switch (m_recordingMode) {
            case LightSensorManager::IndependentMode:
                m_saver->saveDataAsync(data);
                break;
            case LightSensorManager::BatchMode:
                emit dataReady(data);
                break;
        }
    }
}

void LightSensorManager::msToAtimeAstep(int ms, int &atime, int &astep) const {
    // Опорная точка: atime = 10, astep = 20000 при T = 556 мс
    const int refAtime = 10;
    const int refAstep = 20000;
    const int refMs = 556;

    // Вычисляем желаемый astep линейно по времени экспозиции
    double astepDouble = static_cast<double>(refAstep + 1) * ms / refMs - 1.0;
    astep = static_cast<int>(astepDouble + 0.5);  // округление
    if (astep < 0) astep = 0;
    if (astep > 65535) astep = 65535;
    atime = refAtime;
}
