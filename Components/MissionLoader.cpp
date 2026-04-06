#include "MissionLoader.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

bool MissionLoader::load(const QString &filePath, MissionTask &task) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open mission file:" << filePath;
        return false;
    }
    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        qWarning() << "Invalid JSON in mission file";
        return false;
    }
    QJsonObject obj = doc.object();

    task.name = obj["mission_name"].toString();
    task.version = obj["version"].toInt();

    // Парсинг area
    QJsonObject areaObj = obj["area"].toObject();
    task.area.type = areaObj["type"].toString();
    QJsonArray coords = areaObj["coordinates"].toArray();
    task.area.coordinates.clear();
    for (auto val : coords)
        task.area.coordinates.append(val.toDouble());
    task.area.coordinateSystem = areaObj["coordinate_system"].toString();

    // KML reference
    if (obj.contains("kml_reference") && obj["kml_reference"].isObject()) {
        QJsonObject kmlObj = obj["kml_reference"].toObject();
        task.area.kml.file = kmlObj["file"].toString();
        task.area.kml.polygonName = kmlObj["polygon_name"].toString();
    }

    task.flightAltitudeM = obj["flight_altitude_m"].toInt();
    task.overlapPercent = obj["overlap_percent"].toInt();
    task.flightSpeedMps = obj["flight_speed_mps"].toDouble();
    task.settingsPreset = obj["settings_preset"].toString();

    // camera_settings
    QJsonObject camObj = obj["camera_settings"].toObject();
    task.cameraSettings.hsExposureMs = camObj["hs_exposure_ms"].toInt();
    task.cameraSettings.hsFramerate = camObj["hs_framerate"].toInt();
    task.cameraSettings.ocExposureMs = camObj["oc_exposure_ms"].toInt();
    task.cameraSettings.ocFramerate = camObj["oc_framerate"].toInt();

    // light_sensor_settings
    QJsonObject lightObj = obj["light_sensor_settings"].toObject();
    task.lightSensorSettings.exposureMs = lightObj["exposure_ms"].toInt();
    task.lightSensorSettings.framerate = lightObj["framerate"].toInt();

    // recording
    QJsonObject recObj = obj["recording"].toObject();
    task.recording.startAltitudeM = recObj["start_altitude_m"].toInt();
    task.recording.saveRawData = recObj["save_raw_data"].toBool();
    task.recording.saveClassification = recObj["save_classification"].toBool();

    task.classificationModel = obj["classification_model"].toString();
    task.autoStart = obj["auto_start"].toBool();

    return true;
}

bool MissionLoader::save(const QString &filePath, const MissionTask &task) {
    QJsonObject obj;
    obj["mission_name"] = task.name;
    obj["version"] = task.version;

    // area
    QJsonObject areaObj;
    areaObj["type"] = task.area.type;
    QJsonArray coords;
    for (double v : task.area.coordinates)
        coords.append(v);
    areaObj["coordinates"] = coords;
    areaObj["coordinate_system"] = task.area.coordinateSystem;
    obj["area"] = areaObj;

    // kml_reference
    QJsonObject kmlObj;
    kmlObj["file"] = task.area.kml.file;
    kmlObj["polygon_name"] = task.area.kml.polygonName;
    obj["kml_reference"] = kmlObj;

    obj["flight_altitude_m"] = task.flightAltitudeM;
    obj["overlap_percent"] = task.overlapPercent;
    obj["flight_speed_mps"] = task.flightSpeedMps;
    obj["settings_preset"] = task.settingsPreset;

    // camera_settings
    QJsonObject camObj;
    camObj["hs_exposure_ms"] = task.cameraSettings.hsExposureMs;
    camObj["hs_framerate"] = task.cameraSettings.hsFramerate;
    camObj["oc_exposure_ms"] = task.cameraSettings.ocExposureMs;
    camObj["oc_framerate"] = task.cameraSettings.ocFramerate;
    obj["camera_settings"] = camObj;

    // light_sensor_settings
    QJsonObject lightObj;
    lightObj["exposure_ms"] = task.lightSensorSettings.exposureMs;
    lightObj["framerate"] = task.lightSensorSettings.framerate;
    obj["light_sensor_settings"] = lightObj;

    // recording
    QJsonObject recObj;
    recObj["start_altitude_m"] = task.recording.startAltitudeM;
    recObj["save_raw_data"] = task.recording.saveRawData;
    recObj["save_classification"] = task.recording.saveClassification;
    obj["recording"] = recObj;

    obj["classification_model"] = task.classificationModel;
    obj["auto_start"] = task.autoStart;

    QJsonDocument doc(obj);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) return false;
    file.write(doc.toJson());
    return true;
}