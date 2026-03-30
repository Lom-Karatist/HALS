#ifndef HALSFACADE_H
#define HALSFACADE_H

#include <QObject>
#include <memory>

#include "CameraManager.h"
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
    bool isFlashMounted() const;  // пока заглушка
    bool isHSCameraReady() const;
    bool isOCCameraReady() const;
    bool isLightSensorReady() const;  // заглушка
    bool isMissionLoaded() const;     // заглушка

signals:
    void overviewImageReady(const QImage &image);
    void cameraError(const QString &error);
    void gpsSatellitesCountUpdated(const int &count);

private slots:
    void onGpsDataUpdated(const GpsData &gpsData);

private:
    void initialize();
    bool startCameras();
    bool startGps();

    std::unique_ptr<CameraManager> m_cameraManager;
    std::unique_ptr<GPSDevice> m_gpsDevice;

    int m_satellitesCount;
};

#endif  // HALSFACADE_H
