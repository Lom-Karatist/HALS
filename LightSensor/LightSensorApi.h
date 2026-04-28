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

    enum Channel {
        CH_F1,
        CH_F2,
        CH_F3,
        CH_F4,
        CH_F5,
        CH_F6,
        CH_F7,
        CH_F8,
        CH_NIR,
        CH_CLEAR,
        CH_FD
    };

    // Регистры AS7341 (ключевые)
    static const uint8_t REG_ENABLE = 0x80;
    static const uint8_t REG_ATIME = 0x81;
    static const uint8_t REG_WTIME = 0x83;
    static const uint8_t REG_ASTEP_L = 0xCA;
    static const uint8_t REG_ASTEP_H = 0xCB;
    static const uint8_t REG_CFG0 = 0xA9;
    static const uint8_t REG_CFG1 = 0xAA;
    static const uint8_t REG_CFG6 = 0xAF;
    static const uint8_t REG_CFG9 = 0xB2;
    static const uint8_t REG_SMUX_CMD = 0xAF;
    static const uint8_t REG_STATUS = 0x71;
    static const uint8_t REG_ID = 0x92;
    static const uint8_t REG_ASTATUS = 0x94;
    static const uint8_t REG_CH0_L = 0x95;
    static const uint8_t REG_CH0_H = 0x96;
    static const uint8_t REG_CH1_L = 0x97;
    static const uint8_t REG_CH1_H = 0x98;
    static const uint8_t REG_CH2_L = 0x99;
    static const uint8_t REG_CH2_H = 0x9A;
    static const uint8_t REG_CH3_L = 0x9B;
    static const uint8_t REG_CH3_H = 0x9C;
    static const uint8_t REG_CH4_L = 0x9D;
    static const uint8_t REG_CH4_H = 0x9E;
    static const uint8_t REG_CH5_L = 0x9F;
    static const uint8_t REG_CH5_H = 0xA0;

    // SMUX конфигурации (проверенные, из даташита и библиотеки Adafruit)
    // Цикл 0: F1, F2, F3, F4, NIR, CLEAR
    static const uint8_t SMUX_CONFIG_CYCLE0[16];
    // Цикл 1: F5, F6, F7, F8, FD, CLEAR
    static const uint8_t SMUX_CONFIG_CYCLE1[16];

    int m_i2cFd;  //!< Дескриптор I2C-устройства (Linux) или -1.
    int m_integrationTimeMs;  //!< Текущее время интеграции (мс).
    int m_gainIndex;     //!< Текущий индекс усиления.
    bool m_initialized;  //!< Флаг успешной инициализации.
    bool m_emulationMode;  //!< Флаг эмуляции (Windows или принудительно).
};

#endif  // LIGHTSENSORAPI_H
