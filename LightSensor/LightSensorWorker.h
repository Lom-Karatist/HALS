#ifndef LIGHTSENSORWORKER_H
#define LIGHTSENSORWORKER_H

#include <QAtomicInt>
#include <QMutex>
#include <QObject>
#include <QRunnable>
#include <memory>
#include <vector>

#include "LightSensorApi.h"
#include "LightTypes.h"

/**
 * @brief Рабочий класс для опроса датчика освещённости AS7341 в отдельном
 * потоке.
 *
 * Наследует QObject (для сигналов/слотов) и QRunnable (для выполнения в
 * QThreadPool). Реализует бесконечный цикл с заданной частотой, чтение данных с
 * датчика и асинхронную обработку команд изменения параметров через очередь.
 *
 * Класс полностью изолирует блокирующие I2C-операции от главного потока,
 * что предотвращает зависания GUI. Управление параметрами (время интеграции,
 * усиление, частота) происходит через потокобезопасную очередь команд.
 *
 * @see LightSensorManager
 * @see LightSensorApi
 */
class LightSensorWorker : public QObject, public QRunnable {
    Q_OBJECT

public:
    /**
     * @brief Конструктор.
     * @param parent Родительский QObject (может быть nullptr).
     *
     * Инициализирует внутренний API датчика, устанавливает параметры по
     * умолчанию:
     * - время интеграции: 50 мс
     * - индекс усиления: 6 (32x)
     * - частота опроса: 10 Гц
     *
     * Создаёт объект LightSensorApi, но не выполняет его инициализацию
     * (это происходит в методе run() после старта потока).
     */
    explicit LightSensorWorker(QObject *parent = nullptr);

    /**
     * @brief Деструктор.
     *
     * Останавливает рабочий цикл (устанавливает m_isActive = false)
     * и освобождает ресурсы LightSensorApi.
     */
    ~LightSensorWorker();

    /**
     * @brief Точка входа при запуске в потоке (реализация QRunnable).
     *
     * Выполняется в потоке из QThreadPool. Последовательность действий:
     * 1. Инициализация LightSensorApi.
     * 2. При успехе – испускает сигнал connectionStatusChanged(true)
     *    и settingsChanged() с текущими параметрами.
     * 3. Входит в бесконечный цикл с проверкой флага m_isActive.
     * 4. На каждой итерации:
     *    - обрабатывает ожидающие команды (applyPendingCommands())
     *    - вычисляет задержку в соответствии с частотой m_frameRateHz
     *    - вызывает readAllChannels()
     *    - при успехе испускает dataReady()
     *    - выдерживает паузу для обеспечения заданной частоты
     *
     * Цикл завершается при установке m_isActive = false (вызов stopWork()).
     */
    void run() override;

    /**
     * @brief Активировать рабочий цикл.
     *
     * Устанавливает флаг m_isActive в true, после чего поток, выполняющий
     * run(), начинает опрос датчика. Обычно вызывается перед запуском
     * QRunnable.
     */
    void activate();

    /**
     * @brief Остановить рабочий цикл.
     *
     * Устанавливает флаг m_isActive в false, что приводит к выходу из цикла
     * run() и завершению потока. После вызова этого метода объект нельзя
     * переиспользовать.
     */
    void stopWork();

    /**
     * @brief Установить время интеграции (экспозиции).
     * @param ms Время в миллисекундах (1..1000).
     *
     * Команда помещается в очередь и будет выполнена в рабочем потоке
     * при ближайшей итерации цикла. После применения новое значение
     * сохраняется в m_integrationTimeMs и передаётся в LightSensorApi.
     */
    void setIntegrationTimeMs(int ms);

    /**
     * @brief Установить усиление по индексу.
     * @param index Индекс от 0 до 10 (0:0.5x, 1:1x, 2:2x, 3:4x, 4:8x, 5:16x,
     *              6:32x, 7:64x, 8:128x, 9:256x, 10:512x).
     *
     * Команда помещается в очередь и будет выполнена в рабочем потоке.
     */
    void setGainIndex(int index);

    /**
     * @brief Установить частоту опроса датчика.
     * @param hz Частота в герцах (1..60).
     *
     * Команда помещается в очередь. Новое значение влияет на интервал
     * ожидания между измерениями в следующей итерации цикла.
     */
    void setFrameRateHz(int hz);

    /**
     * @brief Получить текущее время интеграции (из кэша рабочего потока).
     * @return Время интеграции в миллисекундах.
     *
     * Значение может не соответствовать реальному состоянию датчика,
     * если команда ещё не выполнена. Для синхронизации используйте сигнал
     * settingsChanged().
     */
    int integrationTimeMs() const;

    /**
     * @brief Получить текущий индекс усиления (из кэша рабочего потока).
     * @return Индекс от 0 до 10.
     */
    int gainIndex() const;

    /**
     * @brief Получить текущую целевую частоту опроса (из кэша рабочего потока).
     * @return Частота в герцах.
     */
    int frameRateHz() const;

signals:
    /**
     * @brief Сигнал, испускаемый при каждом успешном измерении.
     * @param data Структура с каналами, временной меткой, параметрами
     *             и углом Солнца (если передан через updateSunElevation).
     *
     * Испускается в рабочем потоке. При подключении с AutoConnection
     * Qt автоматически переносит вызов в поток получателя.
     */
    void dataReady(const LightSensorData &data);

    /**
     * @brief Сигнал об ошибке при работе с датчиком.
     * @param msg Текст ошибки (например, "Failed to open I2C bus").
     */
    void errorOccurred(const QString &msg);

    /**
     * @brief Сигнал об изменении состояния подключения датчика.
     * @param connected true – инициализация успешна, датчик готов.
     *
     * Испускается один раз после вызова run().
     */
    void connectionStatusChanged(bool connected);

    /**
     * @brief Сигнал об изменении параметров датчика.
     * @param params Структура с новыми значениями (экспозиция, усиление,
     * частота).
     *
     * Испускается после инициализации и после применения любой команды
     * из очереди (SetIntegrationTime, SetGainIndex, SetFrameRate).
     */
    void settingsChanged(const LightSensorParameters &params);

private:
    /**
     * @brief Обработать все накопленные команды из очереди.
     *
     * Вызывается в рабочем потоке в начале каждой итерации цикла,
     * если флаг m_isCommandsPending установлен.
     * Последовательно применяет команды, обновляя m_integrationTimeMs,
     * m_gainIndex, m_frameRateHz и вызывая соответствующие методы
     * LightSensorApi. После применения всех команд испускает сигнал
     * settingsChanged().
     *
     * @note Мьютекс m_cmdMutex защищает очередь на время перемещения команд.
     */
    void applyPendingCommands();
    std::unique_ptr<LightSensorApi>
        m_api;  //!< Низкоуровневый API для работы с датчиком.
    std::atomic<bool>
        m_isActive;  //!< Флаг активности потока (true – выполнять цикл).

    int m_integrationTimeMs;  //!< Текущее время интеграции (кэш, мс).
    int m_gainIndex;  //!< Текущий индекс усиления (кэш, 0..10).
    int m_frameRateHz;  //!< Текущая целевая частота опроса (кэш, Гц).

    QMutex m_cmdMutex;  //!< Мьютекс для защиты очереди команд.
    std::vector<LightCommand>
        m_pendingCommands;  //!< Очередь ожидающих выполнения команд.
    std::atomic<bool> m_isCommandsPending;  //!< Флаг наличия команд в очереди.
};

#endif  // LIGHTSENSORWORKER_H
