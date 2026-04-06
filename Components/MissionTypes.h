#ifndef MISSIONTYPES_H
#define MISSIONTYPES_H

#include <QString>
#include <QVector>

struct KmlReference {
    QString file;
    QString polygonName;
};

struct Area {
    QString type;                 // "rectangle", "polygon", "kml"
    QVector<double> coordinates;  // [lon1,lat1, lon2,lat2] для rectangle
    QString coordinateSystem;     // "WGS84"
    KmlReference kml;
};

struct CameraSettings {
    int hsExposureMs = 10;
    int hsFramerate = 20;
    int ocExposureMs = 10;
    int ocFramerate = 30;
};

struct LightSensorSettings {
    int exposureMs = 50;
    int framerate = 10;
};

struct RecordingSettings {
    int startAltitudeM = 50;
    bool saveRawData = true;
    bool saveClassification = true;
};

struct MissionTask {
    QString name;
    int version = 1;
    Area area;
    int flightAltitudeM = 150;
    int overlapPercent = 70;
    double flightSpeedMps = 5.0;
    QString settingsPreset;
    CameraSettings cameraSettings;
    LightSensorSettings lightSensorSettings;
    RecordingSettings recording;
    QString classificationModel;
    bool autoStart = true;
};

#endif  // MISSIONTYPES_H
