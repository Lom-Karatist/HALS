#include "ExperimentController.h"

#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

#include "CameraManager.h"
#include "DataSaver.h"
#include "FlightTaskModule.h"
#include "gps_device.h"
#include "spa.h"

ExperimentController::ExperimentController(QObject *parent)
    : QObject(parent),
      m_cameraManager(nullptr),
      m_flightTaskModule(nullptr),
      m_dataSaver(nullptr),
      m_experimentActive(false),
      m_monitoring(false),
      m_sunElevation(0.0) {
    m_checkTimer = new QTimer(this);
    connect(m_checkTimer, &QTimer::timeout, this,
            &ExperimentController::checkExperimentCondition);
}

void ExperimentController::setCameraManager(CameraManager *manager) {
    m_cameraManager = manager;
}

void ExperimentController::setFlightTaskModule(FlightTaskModule *module) {
    m_flightTaskModule = module;
}

void ExperimentController::setDataSaver(DataSaver *saver) {
    m_dataSaver = saver;
}

void ExperimentController::setLightSensor(LightSensorManager *sensor) {
    m_lightSensor = sensor;
}

bool ExperimentController::loadMission(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        emit errorOccurred("Cannot open mission file: " + filePath);
        return false;
    }
    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        emit errorOccurred("Invalid JSON in mission file");
        return false;
    }
    QJsonObject obj = doc.object();
    // Загружаем параметры задания
    m_mission.minAltitude = obj.value("minAltitude").toDouble(0.0);
    m_mission.maxAltitude = obj.value("maxAltitude").toDouble(1000.0);
    m_mission.usePolygon = obj.value("usePolygon").toBool(false);
    // Для полигона нужно загрузить координаты, пока упрощённо
    // TODO: загрузка полигона
    return true;
}

void ExperimentController::startMonitoring() {
    if (m_monitoring) return;
    m_monitoring = true;
    m_checkTimer->start(1000);  // проверка каждую секунду
    qDebug() << "Experiment monitoring started";
}

void ExperimentController::stopMonitoring() {
    if (!m_monitoring) return;
    m_checkTimer->stop();
    m_monitoring = false;
    // Если эксперимент был активен, останавливаем запись
    if (m_experimentActive) {
        forceStopExperiment();
    }
    qDebug() << "Experiment monitoring stopped";
}

void ExperimentController::forceStartExperiment() {
    if (m_experimentActive) return;
    setDataRecording(true);
    m_experimentActive = true;
    emit experimentStarted();
    qDebug() << "Experiment forced start";
}

void ExperimentController::forceStopExperiment() {
    if (!m_experimentActive) return;
    setDataRecording(false);
    m_experimentActive = false;
    emit experimentStopped();
    qDebug() << "Experiment forced stop";
}

void ExperimentController::updateGpsData(const GpsData &gpsData) {
    m_currentGpsData = gpsData;

    QDateTime dt = QDateTime::currentDateTime();
    spa_data spa;
    spa.year = dt.date().year();
    spa.month = dt.date().month();
    spa.day = dt.date().day();
    spa.hour = dt.time().hour();
    spa.minute = dt.time().minute();
    spa.second = dt.time().second();
    spa.timezone = 3.0;
    spa.delta_ut1 = 0;
    spa.delta_t = 67;
    spa.longitude = m_currentGpsData.longitude;
    spa.latitude = m_currentGpsData.latitude;
    spa.elevation = 156;
    spa.pressure = 1013;
    spa.temperature = 15;
    spa.slope = 0;
    spa.azm_rotation = 0;
    spa.atmos_refract = 0.5667;
    spa.function = SPA_ZA;

    spa_calculate(&spa);
    m_sunElevation = spa.e;

    emit sunElevationUpdated(m_sunElevation);
}

void ExperimentController::checkExperimentCondition() {
    if (!m_monitoring) return;

    bool shouldStart = shouldStartExperiment();
    bool shouldStop = shouldStopExperiment();

    if (shouldStart && !m_experimentActive) {
        setDataRecording(true);
        m_experimentActive = true;
        emit experimentStarted();
        qDebug() << "Experiment auto-started at altitude"
                 << m_currentGpsData.altitude;
    } else if (shouldStop && m_experimentActive) {
        setDataRecording(false);
        m_experimentActive = false;
        emit experimentStopped();
        qDebug() << "Experiment auto-stopped at altitude"
                 << m_currentGpsData.altitude;
    }
}

bool ExperimentController::shouldStartExperiment() const {
    // Базовые условия: есть достаточное количество спутников (>=4) и высота в
    // допустимом диапазоне
    if (m_currentGpsData.satellites < 4) return false;
    if (m_currentGpsData.altitude < m_mission.minAltitude ||
        m_currentGpsData.altitude > m_mission.maxAltitude)
        return false;

    // Если используется полигон, проверить вхождение точки
    if (m_mission.usePolygon) {
        // TODO: проверка принадлежности m_currentCoord полигону
        // Пока возвращаем true
        return true;
    }
    return true;
}

bool ExperimentController::shouldStopExperiment() const {
    // Останавливаем, если условия перестали выполняться
    //    if (m_currentGpsData.satellites < 3) return true;  // потеря GPS
    if (m_currentGpsData.altitude < m_mission.minAltitude ||
        m_currentGpsData.altitude > m_mission.maxAltitude)
        return true;
    if (m_mission.usePolygon) {
        // TODO: если вышли за пределы полигона, остановить
        return false;
    }
    return false;
}

void ExperimentController::setDataRecording(bool enabled) {
    if (!m_cameraManager) return;

    m_cameraManager->setIsNeedToSave(enabled, enabled, enabled);

    if (m_lightSensor) {
        if (enabled) {
            m_lightSensor->setRecordingEnabled(true);
        } else {
            m_lightSensor->setRecordingEnabled(false);
        }
    }
}

bool ExperimentController::experimentActive() const {
    return m_experimentActive;
}
