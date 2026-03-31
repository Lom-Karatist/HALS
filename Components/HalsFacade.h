#ifndef HALSFACADE_H
#define HALSFACADE_H

#include <QObject>
#include <memory>

#include "CameraManager.h"
#include "CpuTemperatureController.h"
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

    void setSaveFormat(int format);  // 0 - BMP, 1 - Binary

    // Статусные индикаторы (для GUI)
    bool isHSCameraReady() const;
    bool isOCCameraReady() const;
    bool isLightSensorReady() const;  // заглушка
    bool isMissionLoaded() const;     // заглушка

signals:
    void masterConnectionStatusChanged(bool connectionStatus);
    void slaveConnectionStatusChanged(bool connectionStatus);
    void overviewImageReady(const QImage &image);
    void hsImageReady(const QImage &image);
    void cameraError(const QString &error);
    void gpsSatellitesCountUpdated(const int &count);
    void cpuTemperatureUpdated(QString temperature);
    void logMessage(Logger::LogLevel level, const QString &message);
    void usbStatusChanged(bool mounted, qint64 availableBytes,
                          qint64 totalBytes);

private slots:
    void onGpsDataUpdated(const GpsData &gpsData);
    void startBaslerCameras();
    void updateHsData(const QByteArray &data, int w, int h, int pixelFormat);

private:
    void startLogger();
    void startTempController();
    void startUsbChecker();
    bool initCameras();
    bool startGps();

    std::unique_ptr<CameraManager> m_cameraManager;

    std::unique_ptr<GPSDevice> m_gpsDevice;
    int m_satellitesCount;

    std::unique_ptr<Logger> m_logger;
    QThread *m_loggerThread;

    std::unique_ptr<CpuTemperatureController> m_tempController;
    QThread *m_tempControllerThread;

    std::unique_ptr<UsbChecker> m_usbChecker;
};

#endif  // HALSFACADE_H
