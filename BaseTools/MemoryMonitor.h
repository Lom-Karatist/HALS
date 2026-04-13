#ifndef MEMORYMONITOR_H
#define MEMORYMONITOR_H

#include <QObject>
#include <QTimer>

/**
 * @brief Мониторинг использования оперативной памяти.
 *
 * Периодически считывает информацию об ОЗУ (всего, доступно, занято)
 * через системные вызовы (/proc/meminfo на Linux, GlobalMemoryStatusEx на Windows)
 * и испускает сигнал memoryUsageUpdated.
 */
class MemoryMonitor : public QObject {
    Q_OBJECT
public:
    explicit MemoryMonitor(QObject *parent = nullptr);
    ~MemoryMonitor();

    /**
     * @brief Запустить мониторинг с заданным интервалом.
     * @param intervalMs Интервал в миллисекундах (по умолчанию 5000 мс = 5 сек)
     */
    void startMonitoring(int intervalMs = 5000);

    /**
     * @brief Остановить мониторинг.
     */
    void stopMonitoring();

    /**
     * @brief Выполнить однократную проверку и испустить сигнал.
     */
    void check();

signals:
    /**
     * @brief Сигнал с текущим использованием памяти.
     * @param totalBytes Общий объём ОЗУ в байтах.
     * @param availableBytes Доступно байт.
     * @param usedBytes Занято байт (total - available).
     */
    void memoryUsageUpdated(qint64 totalBytes, qint64 availableBytes, qint64 usedBytes);

private slots:
    void onTimer();

private:
    /**
     * @brief Считать текущее использование памяти из системы.
     * @param totalBytes [out]
     * @param availableBytes [out]
     * @return true при успехе.
     */
    bool readSystemMemory(qint64 &totalBytes, qint64 &availableBytes);

    QTimer *m_timer;
};

#endif // MEMORYMONITOR_H