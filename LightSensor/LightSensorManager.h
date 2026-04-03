#ifndef LIGHTSENSORMANAGER_H
#define LIGHTSENSORMANAGER_H

#include <QObject>
#include <QTimer>
#include <memory>

#include "LightSaver.h"
#include "LightSensorApi.h"
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

    /**
     * @brief Инициализировать менеджер и датчик.
     *
     * Вызывает инициализацию LightSensorApi, загружает настройки из INI-файла
     * и применяет их к датчику. После успешной инициализации испускается
     * сигнал settingsChanged() с текущими параметрами.
     */
    void initialize();

    /**
     * @brief Запустить периодический опрос датчика.
     *
     * Таймер запускается с интервалом, соответствующим установленной частоте
     * (frameRateHz). Если опрос уже активен, ничего не делает.
     */
    void start();

    /**
     * @brief Остановить периодический опрос датчика.
     */
    void stop();

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

private slots:
    /**
     * @brief Слот, вызываемый по таймеру для выполнения одного измерения.
     *
     * Читает данные через LightSensorApi, при успехе испускает dataReady()
     * и, если сохранение включено, вызывает асинхронное сохранение.
     */
    void onTimer();

private:
    std::unique_ptr<LightSensorApi>
        m_api;  //!< Низкоуровневый API для работы с датчиком.
    std::unique_ptr<LightSettings>
        m_lightSettings;  //!< Настройки датчика (INI-файл).
    std::unique_ptr<LightSaver> m_saver;  //!< Модуль сохранения данных.
    QTimer *m_timer;  //!< Таймер для периодического опроса.
    bool m_isActive;  //!< Флаг активности опроса.
};

#endif  // LIGHTSENSORMANAGER_H
