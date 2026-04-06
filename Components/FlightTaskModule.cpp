#include "FlightTaskModule.h"

#include <QDebug>
#include <cmath>

#include "CameraManager.h"
#include "LightSensorManager.h"
#include "MissionLoader.h"

FlightTaskModule::FlightTaskModule(QObject *parent, int ocSensorWidth,
                                   int hsSensorHeight)
    : QObject(parent),
      m_missionValid(false),
      m_flightAltitude(2),
      m_ocSensorWidth(ocSensorWidth),
      m_hsSensorHeight(hsSensorHeight) {
    recalculateCamerasChars();
}

FlightTaskModule::~FlightTaskModule() {}

void FlightTaskModule::loadMission(QString dirPath) {
    dirPath.append("HALS/");

    QDir dir(dirPath);
    if (!dir.exists()) {
        qDebug() << "Directory not found:" << dirPath;
        emit missionLoaded(false);
        return;
    }

    QStringList filters;
    filters << "*.json";
    QStringList jsonFiles = dir.entryList(filters, QDir::Files);
    if (jsonFiles.isEmpty()) {
        qDebug() << "No JSON files found in" << dirPath;
        emit missionLoaded(false);
        return;
    }

    QString missionFilePath = dirPath + "/" + jsonFiles.first();
    qDebug() << "Loading mission from:" << missionFilePath;

    if (!MissionLoader::load(missionFilePath, m_mission)) {
        m_missionValid = false;
        emit missionLoaded(false);
        return;
    }

    m_missionValid = true;
    emit missionLoaded(true);
}

void FlightTaskModule::applyMissionSettings() {
    if (!m_missionValid) return;

    setAltitude(m_mission.flightAltitudeM);
}

void FlightTaskModule::setAltitude(int altitude) {
    if (m_flightAltitude != altitude) {
        m_flightAltitude = altitude;
        emit altitudeWasUpdated(altitude);
        recalculateCamerasChars();
    }
}

int FlightTaskModule::flightAltitude() const { return m_flightAltitude; }

void FlightTaskModule::updateChars() { recalculateCamerasChars(); }

void FlightTaskModule::recalculateCamerasChars() {
    double fovMeters, gsd;
    recalculateSingleCameraChars(m_ocFovWidthDegree, m_ocSensorWidth, fovMeters,
                                 gsd);
    emit ocCharsWereUpdated(fovMeters, gsd);
    recalculateSingleCameraChars(m_hsFovWidthDegree, m_hsSensorHeight,
                                 fovMeters, gsd);
    emit hsCharsWereUpdated(fovMeters, gsd);
}

void FlightTaskModule::recalculateSingleCameraChars(int fovDegree,
                                                    int sensorSizePx,
                                                    double &fovMeters,
                                                    double &gsd) {
    fovMeters = 2 * m_flightAltitude * tan(M_PI * fovDegree / (2 * 180));
    gsd = fovMeters * 1000 / sensorSizePx;
}

void FlightTaskModule::onAltitudeChanged(int altitude) {
    Q_UNUSED(altitude)
    // Можно добавить дополнительную логику при изменении высоты из GUI
}
