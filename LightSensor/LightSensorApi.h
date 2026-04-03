#ifndef LIGHTSENSORAPI_H
#define LIGHTSENSORAPI_H

#include <QObject>

#include "LightTypes.h"

/**
 * @brief Класс для низкоуровневого управления датчиком освещённости AS7341
 * через I2C.
 *
 * Обеспечивает инициализацию, настройку времени интеграции и усиления,
 * а также чтение всех 11 спектральных каналов. Для полноценной работы
 * на Linux требуется поддержка I2C в ядре и права доступа к /dev/i2c-1.
 * На Windows используется эмуляция (генерация случайных данных) для разработки.
 *
 * Чтение всех каналов выполняется за два цикла измерения с переключением
 * конфигурации SMUX: первый цикл – каналы F1, F2, F3, F4, NIR, CLEAR;
 * второй цикл – F5, F6, F7, F8, FD, CLEAR (значение CLEAR усредняется).
 */
class LightSensorApi : public QObject {
    Q_OBJECT
public:
    /**
     * @brief Конструктор.
     * @param parent Родительский QObject.
     */
    explicit LightSensorApi(QObject *parent = nullptr);

    /**
     * @brief Деструктор. Закрывает I2C-дескриптор, если он был открыт.
     */
    ~LightSensorApi();

    /**
     * @brief Инициализировать датчик.
     * @return true, если инициализация успешна (или включена эмуляция).
     *
     * На Linux открывает /dev/i2c-1, настраивает питание, загружает
     * SMUX-конфигурации для двух циклов и устанавливает параметры по умолчанию.
     */
    bool initialize();

    /**
     * @brief Установить время интеграции.
     * @param ms Время в миллисекундах (1..1000).
     *
     * Влияет на чувствительность и максимальное значение АЦП.
     */
    void setIntegrationTimeMs(int ms);

    /**
     * @brief Установить усиление по индексу.
     * @param index Индекс от 0 до 10, соответствующий множителю:
     *              0:0.5x, 1:1x, 2:2x, 3:4x, 4:8x, 5:16x,
     *              6:32x, 7:64x, 8:128x, 9:256x, 10:512x.
     */
    void setGainByIndex(int index);

    /**
     * @brief Считать значения всех 11 каналов.
     * @param data [out] Структура для заполнения данными.
     * @return true, если чтение успешно.
     *
     * Выполняет два цикла измерения (с переключением SMUX), усредняет канал
     * CLEAR. При эмуляции генерирует случайные значения.
     */
    bool readAllChannels(LightSensorData &data);

    /**
     * @brief Получить текущее время интеграции.
     * @return Время в миллисекундах.
     */
    int getIntegrationTimeMs() const { return m_integrationTimeMs; }

    /**
     * @brief Получить текущий индекс усиления.
     * @return Индекс (0..10).
     */
    int getGainIndex() const { return m_gainIndex; }

    /**
     * @brief Принудительно включить режим эмуляции (полезно для отладки на
     * Linux без железа).
     * @param enable true – включить эмуляцию.
     */
    void setEmulationMode(bool enable);

signals:
    /**
     * @brief Сигнал об ошибке при работе с датчиком.
     * @param msg Текст ошибки.
     */
    void errorOccurred(const QString &msg);

private:
#ifdef Q_OS_LINUX
    /**
     * @brief Записать один байт в регистр датчика по I2C.
     * @param reg Адрес регистра.
     * @param value Значение для записи.
     * @return true, если операция успешна.
     */
    bool writeRegister(uint8_t reg, uint8_t value);

    /**
     * @brief Прочитать один байт из регистра датчика по I2C.
     * @param reg Адрес регистра.
     * @param value [out] Прочитанное значение.
     * @return true, если операция успешна.
     */
    bool readRegister(uint8_t reg, uint8_t &value);

    /**
     * @brief Прочитать блок байт из последовательных регистров, начиная с
     * указанного.
     * @param reg Начальный адрес регистра.
     * @param buffer Буфер для приёма данных.
     * @param len Количество байт для чтения.
     * @return true, если операция успешна.
     */
    bool readBlock(uint8_t reg, uint8_t *buffer, size_t len);

    /**
     * @brief Записать 16-байтную конфигурацию SMUX в оперативную память
     * датчика.
     * @param config Массив из 16 байт конфигурации.
     * @return true, если операция успешна.
     */
    bool writeSmuxConfig(const uint8_t config[16]);

    /**
     * @brief Переключить SMUX на один из двух предустановленных циклов.
     * @param cycle Номер цикла: 0 или 1.
     * @return true, если переключение выполнено успешно.
     */
    bool switchSmuxCycle(int cycle);

    /**
     * @brief Запустить процесс измерения (установить бит SP_EN).
     * @return true, если команда принята.
     */
    bool startMeasurement();

    /**
     * @brief Ожидать готовности данных от датчика.
     * @param timeoutMs Максимальное время ожидания в миллисекундах.
     * @return true, если данные готовы в течение таймаута.
     */
    bool waitForDataReady(int timeoutMs);
#endif

    /**
     * @brief Эмулировать чтение данных (для Windows или принудительной
     * эмуляции).
     * @param data [out] Структура для заполнения случайными значениями.
     * @return true (всегда успешно).
     */
    bool emulationReadAllChannels(LightSensorData &data);

    int m_i2cFd;  //!< Дескриптор I2C-устройства (Linux) или -1.
    int m_integrationTimeMs;  //!< Текущее время интеграции (мс).
    int m_gainIndex;     //!< Текущий индекс усиления.
    bool m_initialized;  //!< Флаг успешной инициализации.
    bool m_emulationMode;  //!< Флаг эмуляции (Windows или принудительно).
};

#endif  // LIGHTSENSORAPI_H
