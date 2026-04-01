#ifndef CPUTEMPERATURECONTROLLER_H
#define CPUTEMPERATURECONTROLLER_H

#include <QObject>
#include <QTimer>

/**
 * @brief Класс для чтения температуры процессора.
 *
 * Периодически (каждые 10 секунд) считывает температуру CPU через
 * платформозависимые методы:
 * - Linux: чтение /sys/class/thermal/thermal_zone0/temp (значение в
 * миллиградусах).
 * - Windows: запрос к WMI (класс MSAcpi_ThermalZoneTemperature).
 *
 * Результат передаётся через сигнал cpuTemperatureUpdated.
 *
 * @note Для работы на Windows необходима инициализация COM (выполняется внутри
 *       getCpuTemperature). На Linux используется стандартный sysfs.
 */
class CpuTemperatureController : public QObject {
    Q_OBJECT
public:
    /**
     * @brief Конструктор.
     * @param parent Родительский QObject.
     *
     * Запускает таймер с интервалом 10 секунд, по которому вызывается
     * getCpuTemperature().
     */
    explicit CpuTemperatureController(QObject *parent = nullptr);
    ~CpuTemperatureController();

public slots:
    /**
     * @brief Остановить таймер опроса температуры.
     *
     * Вызывается перед завершением потока, чтобы таймер был остановлен
     * в том же потоке, где был создан.
     */
    void stopTimer();

signals:
    /**
     * @brief Сигнал с текущей температурой процессора.
     * @param temperature Температура в градусах Цельсия в виде строки.
     *                    В случае ошибки возвращает строку с кодом ошибки:
     *                    -1, -2, -3, -4, -5 (Windows) или -1 (Linux).
     */
    void cpuTemperatureUpdated(QString temperature);

private slots:
    /**
     * @brief Слот, вызываемый по таймеру для получения температуры.
     *
     * Выполняет чтение температуры в зависимости от ОС и эмитирует
     * cpuTemperatureUpdated.
     */
    void getCpuTemperature();

private:
    QTimer *m_tempTimer;  //!< Таймер для периодического опроса.
};

#endif  // CPUTEMPERATURECONTROLLER_H
