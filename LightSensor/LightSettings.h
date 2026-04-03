#ifndef LIGHTSETTINGS_H
#define LIGHTSETTINGS_H

#include <QObject>
#include <QSettings>

/**
 * @brief Класс для загрузки, сохранения и управления настройками датчика
 * освещённости AS7341.
 *
 * Настройки хранятся в INI-файле (по умолчанию LS.ini, имя файла передаётся в
 * конструктор). Поддерживаются три параметра:
 * - время интеграции (мс),
 * - индекс усиления (0..10, соответствует множителям 0.5x..512x),
 * - частота опроса (Гц).
 *
 * При загрузке значения проверяются на допустимые диапазоны и при необходимости
 * корректируются. Изменение любого параметра автоматически сохраняет его в
 * файл.
 */
class LightSettings : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Конструктор.
     * @param parent Родительский QObject.
     * @param fileName Путь к INI-файлу настроек. Если пустая строка,
     * используется "LS.ini".
     *
     * Создаёт объект QSettings и загружает параметры из файла (или значения по
     * умолчанию).
     */
    explicit LightSettings(QObject *parent = nullptr, QString fileName = "");
    ~LightSettings();

    /**
     * @brief Загрузить настройки из INI-файла.
     *
     * Считывает значения ключей IntegrationTimeMs, GainIndex, FrameRateHz.
     * Если ключи отсутствуют, применяются значения по умолчанию (50, 6, 10).
     * Выполняет проверку границ и корректировку.
     */
    void loadFromFile();

    /**
     * @brief Сохранить текущие настройки в INI-файл.
     *
     * Записывает значения m_integrationTimeMs, m_gainIndex, m_frameRateHz
     * и синхронизирует файл на диске.
     */
    void saveToFile() const;

    /**
     * @brief Получить текущее время интеграции.
     * @return Время интеграции в миллисекундах (1..1000).
     */
    int integrationTimeMs() const;

    /**
     * @brief Установить время интеграции.
     * @param newIntegrationTimeMs Новое значение (1..1000).
     *
     * Автоматически сохраняет изменения в файл.
     */
    void setIntegrationTimeMs(int newIntegrationTimeMs);

    /**
     * @brief Получить текущий индекс усиления.
     * @return Индекс от 0 до 10, соответствующий множителю:
     *         0 → 0.5x, 1 → 1x, 2 → 2x, 3 → 4x, 4 → 8x, 5 → 16x,
     *         6 → 32x, 7 → 64x, 8 → 128x, 9 → 256x, 10 → 512x.
     */
    int gainIndex() const;

    /**
     * @brief Установить индекс усиления.
     * @param newGainIndex Новый индекс (0..10).
     *
     * Автоматически сохраняет изменения в файл.
     */
    void setGainIndex(int newGainIndex);

    /**
     * @brief Получить целевую частоту опроса датчика.
     * @return Частота в герцах (1..60).
     */
    int frameRateHz() const;

    /**
     * @brief Установить частоту опроса датчика.
     * @param newFrameRateHz Новое значение (1..60).
     *
     * Автоматически сохраняет изменения в файл.
     */
    void setFrameRateHz(int newFrameRateHz);

private:
    int m_integrationTimeMs;  //!< Время интеграции в миллисекундах (1..1000)
    int m_gainIndex;    //!< Индекс усиления (0..10)
    int m_frameRateHz;  //!< Частота опроса в Гц (1..60)

    QSettings *m_settings;  //!< Объект для работы с INI-файлом
};

#endif  // LIGHTSETTINGS_H
