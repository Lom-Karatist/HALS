#ifndef LIGHTSENSORMANAGER_H
#define LIGHTSENSORMANAGER_H

#include <QMutex>
#include <QObject>
#include <QProcess>
#include <QTimer>
#include <atomic>
#include <memory>

#include "LightSaver.h"
#include "LightSettings.h"
#include "LightTypes.h"

/**
 * @brief Менеджер датчика освещённости AS7341.
 *
 * Координирует работу низкоуровневого API (LightSensorApi), настроек
 * (LightSettings) и асинхронного сохранения (LightSaver). Обеспечивает
 * периодический опрос датчика с заданной частотой, применение параметров (время
 * интеграции, усиление, частота) и передачу данных через сигналы.
 *
 * Класс управляет таймером, который запускается при вызове start() и
 * останавливается при stop(). При каждом срабатывании таймера читаются все
 * каналы датчика, испускается сигнал dataReady() и, если включено сохранение,
 * данные асинхронно записываются в файл.
 *
 * @see LightSensorApi
 * @see LightSettings
 * @see LightSaver
 */
class LightSensorManager : public QObject {
    Q_OBJECT
public:
    /**
     * @brief Конструктор.
     * @param parent Родительский QObject.
     */
    explicit LightSensorManager(QObject *parent = nullptr);

    /**
     * @brief Деструктор. Останавливает таймер и освобождает ресурсы.
     */
    ~LightSensorManager();

    void startAs7341Stream(int expoMs, int gainIndex, int framerateHz);

    void stopAs7341Stream();

    /**
     * @brief Инициализировать менеджер и датчик.
     *
     * Вызывает инициализацию LightSensorApi, загружает настройки из INI-файла
     * и применяет их к датчику. После успешной инициализации испускается
     * сигнал settingsChanged() с текущими параметрами.
     */
    void initialize();

    /**
     * @brief Установить время интеграции (экспозиции).
     * @param ms Время в миллисекундах (1..1000).
     *
     * Сохраняет значение в настройки и применяет к датчику.
     */
    void setIntegrationTimeMs(int ms);

    /**
     * @brief Установить усиление по индексу.
     * @param index Индекс от 0 до 10 (0.5x .. 512x).
     *
     * Сохраняет значение в настройки и применяет к датчику.
     */
    void setGainIndex(int index);

    /**
     * @brief Установить частоту опроса датчика.
     * @param hz Частота в герцах (1..60).
     *
     * Сохраняет значение в настройки и, если опрос активен, перезапускает
     * таймер с новым интервалом.
     */
    void setFrameRateHz(int hz);

    /**
     * @brief Установить путь для сохранения данных.
     * @param path Каталог, куда будут записываться JSONL-файлы.
     *
     * Передаёт путь модулю LightSaver.
     */
    void setSavingPath(const QString &path);

    /**
     * @brief Включить или выключить сохранение данных датчика.
     * @param enabled true – данные будут сохраняться в JSONL-файл,
     *                false – сохранение отключено.
     */
    void setRecordingEnabled(bool enabled);

signals:
    /**
     * @brief Сигнал, испускаемый при получении новых данных от датчика.
     * @param data Структура с каналами, временной меткой и параметрами.
     */
    void dataReady(const LightSensorData &data);

    /**
     * @brief Сигнал об ошибке при работе с датчиком.
     * @param msg Текст ошибки.
     */
    void errorOccurred(const QString &msg);

    /**
     * @brief Сигнал об изменении параметров датчика.
     * @param params Структура с новыми параметрами (экспозиция, усиление,
     * частота).
     *
     * Испускается после инициализации или при программном изменении параметров
     * через методы setIntegrationTimeMs(), setGainIndex(), setFrameRateHz().
     */
    void settingsChanged(const LightSensorParameters &params);

    /**
     * @brief Сигнал об изменении состояния подключения датчика освещённости.
     * @param connected true – датчик инициализирован и готов к работе,
     *                  false – инициализация не удалась или произошёл сбой.
     *
     * Сигнал испускается один раз после вызова initialize().
     * В будущем может также испускаться при динамическом восстановлении/потере
     * связи.
     */
    void connectionStatusChanged(bool connected);

public slots:
    /**
     * @brief Обновить текущее значение угла Солнца.
     * @param elevation Угол в градусах (0..90).
     *
     * Потокобезопасно сохраняет значение для последующего добавления в данные.
     */
    void updateSunElevation(double elevation);

private slots:
    /**
     * @brief Слот для приёма данных от рабочего потока LightSensorWorker.
     * @param data Структура с измеренными значениями (передаётся по значению,
     *             что безопасно для跨-потоковой передачи, так как
     * LightSensorData содержит QVector, который поддерживает копирование с
     * разделением (implicit sharing)).
     */
    void onDataReady(LightSensorData data);

private:
#ifdef Q_OS_LINUX
    QProcess *m_lsProcess = nullptr;
    QThread *m_udpThread = nullptr;
    UdpLightSensorReader *m_udpReader = nullptr;
#endif

    std::unique_ptr<LightSettings>
        m_lightSettings;  //!< Настройки датчика (INI-файл).
    std::unique_ptr<LightSaver> m_saver;  //!< Модуль сохранения данных.
    std::atomic<double>
        m_currentSunElevation;  //!< Текущий угол возвышения Солнца
};

#endif  // LIGHTSENSORMANAGER_H
