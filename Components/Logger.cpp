#include "Logger.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>

Logger::Logger(QObject *parent)
    : QObject(parent),
      m_consoleOutput(true),
      m_maxFileSize(5 * 1024 * 1024)  // 5 MB
      ,
      m_maxBackupCount(3) {
    QString defaultPath =
        QCoreApplication::applicationDirPath() + "/logs/hals.log";
    setLogFile(defaultPath);
}

Logger::~Logger() {
    if (m_logFile.isOpen()) {
        m_logFile.close();
    }
}

void Logger::setLogFile(const QString &filePath) {
    if (m_logFile.isOpen()) {
        m_logFile.close();
    }
    m_logFilePath = filePath;
    QDir dir = QFileInfo(m_logFilePath).dir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    m_logFile.setFileName(m_logFilePath);
    if (m_logFile.open(QIODevice::WriteOnly | QIODevice::Append |
                       QIODevice::Text)) {
        m_logStream.setDevice(&m_logFile);
    } else {
        qWarning() << "Failed to open log file:" << m_logFilePath;
    }
}

void Logger::setMaxBackupCount(int count) { m_maxBackupCount = count; }

void Logger::log(LogLevel level, const QString &message) {
    QString line = formatMessage(level, message);
    if (!m_logFile.isOpen()) return;

    m_logStream << line << Qt::endl;
    m_logStream.flush();
    rotateLogIfNeeded();
}

void Logger::rotateLogIfNeeded() {
    if (m_logFile.size() < m_maxFileSize) return;

    m_logFile.close();

    // Циклическая ротация
    for (int i = m_maxBackupCount; i > 0; --i) {
        QString oldName =
            m_logFilePath + (i > 1 ? "." + QString::number(i - 1) : "");
        QString newName =
            m_logFilePath + (i > 0 ? "." + QString::number(i) : "");
        QFile::rename(oldName, newName);
    }
    QFile::rename(m_logFilePath, m_logFilePath + ".1");

    m_logFile.setFileName(m_logFilePath);
    if (m_logFile.open(QIODevice::WriteOnly | QIODevice::Append |
                       QIODevice::Text)) {
        m_logStream.setDevice(&m_logFile);
    } else {
        qWarning() << "Failed to reopen log file after rotation";
    }
}

QString Logger::formatMessage(LogLevel level, const QString &message) {
    QString timestamp =
        QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    return QString("[%1] %2: %3").arg(timestamp, levelToString(level), message);
}

QString Logger::levelToString(LogLevel level) const {
    switch (level) {
        case Debug:
            return "DEBUG";
        case Info:
            return "INFO";
        case Warning:
            return "WARN";
        case Error:
            return "ERROR";
        case Critical:
            return "CRIT";
        default:
            return "UNKNOWN";
    }
}
