#ifndef HALSFACADE_H
#define HALSFACADE_H

#include <QObject>
#include <memory>

#include "CameraManager.h"
#include "CpuTemperatureController.h"
#include "DataSaver.h"
#include "ExperimentController.h"
#include "FlightTaskModule.h"
#include "Logger.h"
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

    void startExperiment();
    void stopExperiment();

    // Статусные индикаторы (для GUI)
    bool isLightSensorReady() const;  // заглушка
    bool isMissionLoaded() const;     // заглушка

signals:
    void masterConnectionStatusChanged(bool connectionStatus);
    void slaveConnectionStatusChanged(bool connectionStatus);
    void overviewImageReady(const QImage &image, int maxBrightness);
    void hsImageReady(const QImage &image, int maxBrightness);
    void cameraError(const QString &error);
    void gpsSatellitesCountUpdated(const int &count);
    void cpuTemperatureUpdated(QString temperature);
    void logMessage(Logger::LogLevel level, const QString &message);
    void usbStatusChanged(bool mounted, qint64 availableBytes,
                          qint64 totalBytes);
    void ocCharsWereUpdated(double fovMeters, double gsd);
    void hsCharsWereUpdated(double fovMeters, double gsd);

private slots:
    void onGpsDataUpdated(const GpsData &gpsData);
    void startBaslerCameras();
    void updateHsData(const QByteArray &data, int w, int h, int pixelFormat);

private:
    void startLogger();
    void startTempController();
    void startUsbChecker();
    void initDataSaver();
    bool initCameras();
    bool startGps();
    void initExperimentController();
    void initFlightTaskModule();

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
};

#endif  // HALSFACADE_H
