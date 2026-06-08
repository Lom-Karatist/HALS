#ifndef DEVICES_GPS_SRC_GPS_PARSER_GPS_PARSER_H_
#define DEVICES_GPS_SRC_GPS_PARSER_GPS_PARSER_H_

#include <gps_data.h>

#include <QObject>

/**
 * @file gps_parser.h
 * @brief Парсер NMEA-сообщений GPS (GPGGA и GPRMC) с буферизацией потока.
 */

/**
 * @brief Режим эмиссии сигнала gpsUpdated.
 */
enum class EmitMode {
    BothValid,  //!< Сигнал испускается только если получены валидные GPGGA и
                //!< GPRMC с совпадающим временем
    AnyValid  //!< Сигнал испускается при получении хотя бы одного валидного
              //!< пакета
};

/**
 * @brief Класс для парсинга NMEA-строк GPS-приёмника.
 *
 * Принимает сырые данные из последовательного порта, буферизирует их,
 * выделяет полные строки, разделённые '\\n', и разбирает сообщения
 * GPGGA и GPRMC. Результат накапливается в структуре GpsData и
 * испускается сигнал gpsUpdated при выполнении условий emitMode.
 *
 * Поддерживает неполные данные (поступление частями).
 */
class GPSParser : public QObject {
    Q_OBJECT
    friend class TestsParser;

public:
    /**
     * @brief Конструктор.
     * @param mode Режим эмиссии (BothValid по умолчанию).
     */
    explicit GPSParser(EmitMode mode = EmitMode::BothValid);

public slots:
    /**
     * @brief Обрабатывает порцию сырых данных из последовательного порта.
     *
     * Добавляет данные во внутренний буфер, извлекает полные строки
     * (ограниченные '\\n') и передаёт их на парсинг.
     *
     * @param data Новые байты, полученные от GPS-приёмника.
     */
    void processData(const QByteArray &data);

signals:
    /**
     * @brief Сигнал, испускаемый при получении валидных данных согласно режиму
     * emitMode.
     * @param data Структура с разобранными и проверенными данными.
     */
    void gpsUpdated(const GpsData data, QPrivateSignal);

private:
    /**
     * @brief Разбирает одну полную NMEA-строку (начинающуюся с '$').
     * @param line Строка без завершающих '\\r\\n'.
     */
    void parseLine(const QString line);

    /**
     * @brief Разбирает сообщение GPGGA.
     * @param line Строка сообщения.
     * @param isValid Выходной параметр: true, если пакет валиден (fix quality >
     * 0).
     */
    void parseGGA(const QString &line, bool &isValid);

    /**
     * @brief Разбирает сообщение GPRMC.
     * @param line Строка сообщения.
     * @param rmcTime Выходной параметр: время UTC из RMC.
     * @param isValid Выходной параметр: true, если статус фиксации 'A'.
     */
    void parseRMC(const QString &line, QString &rmcTime, bool &isValid);

    /**
     * @brief Сравнивает время из RMC и GGA с точностью до минут.
     * @param rmcTime Время из RMC (hhmmss.ss).
     * @param ggaTime Время из GGA (hhmmss.ss).
     * @return true, если совпадают часы и минуты.
     */
    bool isSameMoment(const QString &rmcTime, const QString &ggaTime);

    EmitMode emitMode;  //!< Режим эмиссии сигнала
    GpsData data;  //!< Накопленные данные (очищаются после эмиссии)
    QString rmcTime;  //!< Время из последнего валидного RMC
    bool isGGA_Ready = false;  //!< Флаг, что GPGGA получен и валиден
    bool isRMC_Ready = false;  //!< Флаг, что GPRMC получен и валиден
    QByteArray m_buffer;  //!< Буфер для неполных данных
};

#endif  // DEVICES_GPS_SRC_GPS_PARSER_GPS_PARSER_H_
