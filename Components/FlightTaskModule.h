#ifndef FLIGHTTASKMODULE_H
#define FLIGHTTASKMODULE_H

#include <QObject>
#include <QTimer>
#include <memory>

#include "MissionTypes.h"
#include "gps_device.h"

class CameraManager;
class LightSensorManager;

class FlightTaskModule : public QObject {
    Q_OBJECT
public:
    explicit FlightTaskModule(QObject *parent = nullptr,
                              int ocSensorWidth = 1920,
                              int hsSensorHeight = 600);
    ~FlightTaskModule();

    bool loadMission(QString dirPath);

    int flightAltitude() const;
    void updateChars();

    const MissionTask &mission() const;

    int shootingAltitude() const;

public slots:
    void setAltitude(int altitude);

signals:
    void ocCharsWereUpdated(double fovMeters, double gsd);
    void hsCharsWereUpdated(double fovMeters, double gsd);
    void altitudeWasUpdated(int altitude);
    void missionLoaded(bool success);
    void startExperimentRequested();
    void stopExperimentRequested();

private slots:
    void onAltitudeChanged(int altitude);

private:
    void recalculateCamerasChars();
    void recalculateSingleCameraChars(int fovDegree, int sensorSizePx,
                                      double &fovMeters, double &gsd);
    void applyMissionSettings();

    // Данные полётного задания
    MissionTask m_mission;
    bool m_missionValid;

    int m_flightAltitude;
    int m_shootingAltitude;
    int m_ocSensorWidth;
    int m_hsSensorHeight;

    const int m_ocFovWidthDegree = 39;
    const int m_hsFovWidthDegree = 25;
};

#endif  // FLIGHTTASKMODULE_H
