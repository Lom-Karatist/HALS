#ifndef EXPERIMENTCONTROLLER_H
#define EXPERIMENTCONTROLLER_H

#include <QObject>
#include <QTimer>
#include <memory>

#include "Basler/CameraManager.h"
#include "DataSaver.h"
#include "FlightTaskModule.h"
#include "gps_device.h"

/**
 * @brief Управление экспериментом на основе полётного задания и текущих
 * координат.
 */
class ExperimentController : public QObject {
    Q_OBJECT
public:
    explicit ExperimentController(QObject *parent = nullptr);

    // Установка зависимостей (передаются из фасада)
    void setCameraManager(CameraManager *manager);
    void setFlightTaskModule(FlightTaskModule *module);
    void setDataSaver(DataSaver *saver);

    // Загрузка полётного задания из файла JSON
    bool loadMission(const QString &filePath);

    // Запуск/остановка мониторинга эксперимента (включает проверки)
    void startMonitoring();
    void stopMonitoring();

    // Принудительный старт/стоп (для ручного управления)
    void forceStartExperiment();
    void forceStopExperiment();

signals:
    void experimentStarted();
    void experimentStopped();
    void errorOccurred(const QString &error);

public slots:
    void updateGpsData(const GpsData &gpsData);  // приём сырых данных GPS

private slots:
    void checkExperimentCondition();  // периодическая проверка

private:
    // Проверка, нужно ли начать/продолжить запись
    bool shouldStartExperiment() const;
    bool shouldStopExperiment() const;

    // Управление записью данных через CameraManager
    void setDataRecording(bool enabled);

    CameraManager *m_cameraManager;
    FlightTaskModule *m_flightTaskModule;
    DataSaver *m_dataSaver;

    // Состояние эксперимента
    bool m_experimentActive;  // флаг, что эксперимент запущен (идёт запись)
    bool m_monitoring;  // флаг, что ведётся мониторинг условий

    // Текущие данные
    GpsData m_currentGpsData;

    // Параметры полётного задания
    struct Mission {
        //        QGeoCoordinate polygon;  // упрощённо: точка или полигон
        double minAltitude;  // минимальная высота для старта
        double maxAltitude;  // максимальная высота для старта
        bool usePolygon;  // использовать ли полигон (иначе просто по высоте)
        // TODO: добавить полигон (QList<QGeoCoordinate>)
    } m_mission;

    QTimer *m_checkTimer;  // таймер для периодической проверки условий
};

#endif  // EXPERIMENTCONTROLLER_H
