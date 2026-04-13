#include "DataSaver.h"

#include <QDir>
#include <QJsonObject>
#include <QtConcurrent/QtConcurrent>

DataSaver::DataSaver(QObject *parent) : QObject{parent} {}

const QString &DataSaver::savingPath() const {
    QReadLocker locker(&m_pathLock);
    return m_savingPath;
}

void DataSaver::setSavingPath(const QString &newSavingPath) {
    QWriteLocker locker(&m_pathLock);
    m_savingPath = newSavingPath;

    QDir dir(newSavingPath);
    if (!dir.exists()) dir.mkpath(".");
    qDebug() << "in data saver" << newSavingPath;
}

void DataSaver::appendToJsonFileAsync(const QString &filePath,
                                      const QJsonObject &obj) {
    QtConcurrent::run([filePath, obj]() {
        static QMutex mutex;
        QMutexLocker locker(&mutex);

        QDir dir = QFileInfo(filePath).dir();
        if (!dir.exists()) dir.mkpath(".");

        QFile file(filePath);
        if (file.open(QIODevice::Append | QIODevice::Text)) {
            QJsonDocument doc(obj);
            file.write(doc.toJson(QJsonDocument::Compact) + "\n");
            file.close();
        } else {
            qWarning() << "Failed to open JSON file for writing:" << filePath;
        }
    });
}

void DataSaver::onUsbSpaceUpdated(qint64 available, qint64 total) {
    QJsonObject obj;
    obj["timestamp_ms"] = QDateTime::currentMSecsSinceEpoch();
    obj["available_bytes"] = available;
    obj["total_bytes"] = total;
    obj["free_percent"] = (double)available / total * 100.0;
    appendToJsonFileAsync(m_savingPath + "/usb_space.jsonl", obj);
}

void DataSaver::onMemoryUsageUpdated(qint64 totalBytes, qint64 availableBytes,
                                     qint64 usedBytes) {
    QJsonObject obj;
    obj["timestamp_ms"] = QDateTime::currentMSecsSinceEpoch();
    obj["total_mb"] = totalBytes / (1024.0 * 1024.0);
    obj["available_mb"] = availableBytes / (1024.0 * 1024.0);
    obj["used_mb"] = usedBytes / (1024.0 * 1024.0);
    obj["used_percent"] = (double)usedBytes / totalBytes * 100.0;
    appendToJsonFileAsync(m_savingPath + "/ram_space.jsonl", obj);
}
