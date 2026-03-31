#include "FlightTaskModule.h"

#include <QDebug>
#include <cmath>

FlightTaskModule::FlightTaskModule(QObject *parent, int ocSensorWidth,
                                   int hsSensorHeight)
    : QObject{parent} {
    m_ocSensorWidth = ocSensorWidth;
    m_hsSensorHeight = hsSensorHeight;
    m_flightAltitude = 0;
}

void FlightTaskModule::setAltitude(int altitude) {
    if (m_flightAltitude != altitude) {
        m_flightAltitude = altitude;
        recaculateCamerasChars();
    }
}

void FlightTaskModule::recaculateCamerasChars() {
    double fovMeters, gsd;

    recalculateSingleCameraChars(m_ocFovWidthDegree, m_ocSensorWidth, fovMeters,
                                 gsd);
    emit ocCharsWereUpdated(fovMeters, gsd);

    recalculateSingleCameraChars(m_hsFovWidthDegree, m_hsSensorHeight,
                                 fovMeters, gsd);
    emit hsCharsWereUpdated(fovMeters, gsd);
}

void FlightTaskModule::recalculateSingleCameraChars(const int &fovDegree,
                                                    int sensorSizePx,
                                                    double &fovMeters,
                                                    double &gsd) {
    fovMeters = 2 * m_flightAltitude * tan(M_PI * fovDegree / (2 * 180));
    gsd = fovMeters * 1000 / sensorSizePx;
}

void FlightTaskModule::setHsSensorHeight(int newHsSensorHeight) {
    m_hsSensorHeight = newHsSensorHeight;
}

void FlightTaskModule::setOcSensorWidth(int newOcSensorWidth) {
    m_ocSensorWidth = newOcSensorWidth;
}
