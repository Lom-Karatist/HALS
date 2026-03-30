#ifndef LOGGER_H
#define LOGGER_H

#include <QDateTime>
#include <QFile>
#include <QMutex>
#include <QObject>
#include <QString>
#include <QTextStream>
#include <QThread>

class Logger : public QObject {
    Q_OBJECT
public:
    enum LogLevel { Debug, Info, Warning, Error, Critical };
    Q_ENUM(LogLevel)

    explicit Logger(QObject *parent = nullptr);
    ~Logger();

    void setLogFile(const QString &filePath);
    void setMaxBackupCount(int count);  // количество архивных файлов

public slots:
    void log(LogLevel level, const QString &message);

private:
    void rotateLogIfNeeded();
    QString formatMessage(LogLevel level, const QString &message);
    QString levelToString(LogLevel level) const;

    QFile m_logFile;
    QTextStream m_logStream;
    QString m_logFilePath;
    bool m_consoleOutput;
    qint64 m_maxFileSize;
    int m_maxBackupCount;
};

#endif  // LOGGER_H
