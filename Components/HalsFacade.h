#ifndef HALSFACADE_H
#define HALSFACADE_H

#include <QObject>
#include <memory>

#include "CameraManager.h"
#include "CpuTemperatureController.h"
#include "DataSaver.h"
#include "ExperimentController.h"
#include "FlightTaskModule.h"
#include "LightSensor/LightSensorManager.h"
#include "Logger.h"
#include "ParameterTypes.h"
#include "UsbChecker.h"
#include "gps_device.h"

class HalsFacade : public QObject {
    Q_OBJECT

public:
    explicit HalsFacade(QObject *parent = nullptr);
    ~HalsFacade();

    void initialize();
    void refreshUsbState();
    void stopBaslerCameras();
    void setVideoStreamEnabled(bool enabled);

    void setFlightAltitude(int altitude);

    void setSavingPath(QString savingPath);
    void setSaveFormat(int format);  // 0 - BMP, 1 - Binary

    void makeSnapshot();
    void startExperiment();
    void stopExperiment();
    void loadPreset(const QString &presetName);

signals:
    void masterConnectionStatusChanged(bool connectionStatus);
    void slaveConnectionStatusChanged(bool connectionStatus);
    void overviewImageReady(const QImage &image, int maxBrightness);
    void hsImageReady(const QImage &image, int maxBrightness);
    void flightTaskLoaderStatusChanged(bool loadingStatus);
    void cameraError(const QString &error);
    void gpsSatellitesCountUpdated(const int &count);
    void cpuTemperatureUpdated(QString temperature);
    void logMessage(Logger::LogLevel level, const QString &message);
    void usbStatusChanged(bool mounted, qint64 availableBytes,
                          qint64 totalBytes);
    void ocCharsWereUpdated(double fovMeters, double gsd);
    void hsCharsWereUpdated(double fovMeters, double gsd);
    void altitudeWasUpdated(int altitude);
    void shootingAltitudeWasUpdated(int altitude);
    void parameterValueChanged(ParameterType type, int value);

public slots:
    void onParameterChanged(ParameterType type, int newValue);

private slots:
    void onGpsDataUpdated(const GpsData &gpsData);
    void startBaslerCameras();
    void updateHsData(const QByteArray &data, int w, int h, int pixelFormat);
    void onCameraForceParameterChanging(
        bool isMaster, BaslerConstants::SettingTypes settingType,
        QVariant value);
    void onLightForceParameterChanging(const LightSensorParameters &params);
    void onLightDataReady(const LightSensorData &data);
    void onUsbStatusChanged(bool mounted, qint64 availableBytes,
                            qint64 totalBytes);

private:
    void startLogger();
    void startTempController();
    void startUsbChecker();
    void initDataSaver();
    bool initCameras();
    bool startGps();
    void initExperimentController();
    void initFlightTaskModule();
    void initLightSensor();

    std::unique_ptr<CameraManager> m_cameraManager;

    std::unique_ptr<GPSDevice> m_gpsDevice;
    int m_satellitesCount;

    std::unique_ptr<DataSaver> m_dataSaver;
    QThread *m_saverThread;

    std::unique_ptr<ExperimentController> m_experimentController;

    std::unique_ptr<Logger> m_logger;
    QThread *m_loggerThread;

    std::unique_ptr<CpuTemperatureController> m_tempController;
    QThread *m_tempControllerThread;

    std::unique_ptr<UsbChecker> m_usbChecker;

    std::unique_ptr<FlightTaskModule> m_flightTaskModule;

    std::unique_ptr<LightSensorManager> m_lightSensorManager;
};

#endif  // HALSFACADE_H
