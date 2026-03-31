#ifndef LOGGER_H
#define LOGGER_H

#include <QDateTime>
#include <QFile>
#include <QMutex>
#include <QObject>
#include <QString>
#include <QTextStream>
#include <QThread>

/**
 * @brief Класс для асинхронного логирования в файл с ротацией.
 *
 * Предназначен для работы в отдельном потоке. Принимает сообщения через слот
 * log(), форматирует их с временной меткой и уровнем, записывает в файл.
 * При достижении максимального размера файла (по умолчанию 5 МБ) выполняет
 * циклическую ротацию (создаёт .1, .2, ... архивы, количество задаётся).
 *
 * @note Лог-файл открывается при создании объекта или после setLogFile.
 *       Ротация происходит автоматически при каждой записи, если размер
 * превышен.
 */
class Logger : public QObject {
    Q_OBJECT
public:
    enum LogLevel { Debug, Info, Warning, Error, Critical };
    Q_ENUM(LogLevel)

    /**
     * @brief Конструктор.
     * @param parent Родительский QObject.
     *
     * По умолчанию лог-файл создаётся в ./logs/hals.log относительно директории
     * приложения. Устанавливаются размер файла 5 МБ и количество архивов 3.
     */
    explicit Logger(QObject *parent = nullptr);
    ~Logger();

    /**
     * @brief Установить путь к файлу лога.
     * @param filePath Полный путь к файлу.
     *
     * Если файл уже был открыт, он закрывается и открывается новый.
     * Создаётся необходимая директория.
     */
    void setLogFile(const QString &filePath);

    /**
     * @brief Установить количество сохраняемых архивов при ротации.
     * @param count Количество файлов (не включая основной).
     */
    void setMaxBackupCount(int count);

public slots:
    /**
     * @brief Записать сообщение в лог.
     * @param level Уровень логирования.
     * @param message Текст сообщения.
     *
     * Форматирует сообщение, записывает в файл и при необходимости выполняет
     * ротацию. Вызов из любого потока безопасен благодаря Qt::QueuedConnection.
     */
    void log(LogLevel level, const QString &message);

private:
    /**
     * @brief Выполнить ротацию файлов при превышении размера.
     */
    void rotateLogIfNeeded();

    /**
     * @brief Сформировать строку сообщения.
     * @param level Уровень.
     * @param message Текст.
     * @return Форматированная строка вида "[yyyy-MM-dd hh:mm:ss.zzz] УРОВЕНЬ:
     * сообщение".
     */
    QString formatMessage(LogLevel level, const QString &message);

    /**
     * @brief Преобразовать уровень логирования в строку.
     * @param level Уровень.
     * @return Строковое представление (DEBUG, INFO, WARN, ERROR, CRIT).
     */
    QString levelToString(LogLevel level) const;

    QFile m_logFile;          //!< Файл лога.
    QTextStream m_logStream;  //!< Поток для записи в файл.
    QString m_logFilePath;    //!< Путь к файлу лога.
    bool m_consoleOutput;  //!< Не используется (оставлено для совместимости).
    qint64 m_maxFileSize;  //!< Максимальный размер файла в байтах.
    int m_maxBackupCount;  //!< Количество архивных файлов.
};

#endif  // LOGGER_H
