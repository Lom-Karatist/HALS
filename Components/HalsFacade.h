#ifndef HALSFACADE_H
#define HALSFACADE_H

#include <QObject>
#include <memory>

#include "CameraManager.h"
#include "Logger.h"
#include "gps_device.h"

class HalsFacade : public QObject {
    Q_OBJECT

public:
    explicit HalsFacade(QObject *parent = nullptr);
    ~HalsFacade();

    // Инициализация системы

    // Управление захватом
    void startCapture();
    void pauseCapture();
    void stopCapture();

    // Сохранение
    void setSaveFormat(int format);  // 0 - BMP, 1 - Binary

    // Статусные индикаторы (для GUI)
    bool isHSCameraReady() const;
    bool isOCCameraReady() const;
    bool isLightSensorReady() const;  // заглушка
    bool isMissionLoaded() const;     // заглушка

signals:
    void overviewImageReady(const QImage &image);
    void cameraError(const QString &error);
    void gpsSatellitesCountUpdated(const int &count);
    void logMessage(Logger::LogLevel level, const QString &message);

private slots:
    void onGpsDataUpdated(const GpsData &gpsData);

private:
    void initialize();
    void startLogger();
    bool startCameras();
    bool startGps();

    std::unique_ptr<CameraManager> m_cameraManager;

    std::unique_ptr<GPSDevice> m_gpsDevice;
    int m_satellitesCount;

    std::unique_ptr<Logger> m_logger;
    QThread *m_loggerThread;
};

#endif  // HALSFACADE_H
