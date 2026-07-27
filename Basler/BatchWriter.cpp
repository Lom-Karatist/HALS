#include "BatchWriter.h"

#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSysInfo>

BatchWriter::BatchWriter(const QString &basePath, int maxFramesPerBatch,
                         qint64 maxBufferBytesPerBatch, QObject *parent)
    : QObject(parent),
      m_basePath(basePath),
      m_maxFramesPerBatch(maxFramesPerBatch),
      m_maxBufferBytesPerBatch(maxBufferBytesPerBatch),
      m_shutdown(0),
      m_recievedCount(0) {
    QDir dir(m_basePath + "/HS");
    if (!dir.exists()) dir.mkpath(".");
    dir.setPath(m_basePath + "/OC");
    if (!dir.exists()) dir.mkpath(".");
}

BatchWriter::~BatchWriter() { m_shutdown.fetchAndStoreRelease(1); }

void BatchWriter::writeBatch(const QString prefix,
                             const QVector<FrameData> &frames,
                             const QVector<LightSensorData> &lightData) {
    if (m_shutdown.loadAcquire()) return;
    qDebug() << "[RECIEVED]" << QTime::currentTime().toString("hh::mm::ss.zzzz")
             << m_recievedCount;
    writeBatchImpl(prefix, frames, lightData);
}

void BatchWriter::writeBatchImpl(
    const QString &prefix, const QVector<BaslerConstants::FrameData> &frames,
    const QVector<LightSensorData> &lightData) {
    if (frames.isEmpty()) return;

    QString baseName = generateBaseName(prefix);
    qDebug() << m_basePath + "/" + prefix + "/" + baseName + ".json";
    writeBinary(m_basePath + "/" + prefix + "/" + baseName + ".bin", frames);
    writeHeader(m_basePath + "/" + prefix + "/" + baseName + ".json", baseName,
                frames, lightData);

    emit fileWritten(m_basePath + "/" + prefix + "/" + baseName + ".bin",
                     frames.size(), prefix);
}

void BatchWriter::writeBinary(const QString binPath,
                              const QVector<FrameData> &frames) {
    QFile binFile(binPath);
    if (!binFile.open(QIODevice::WriteOnly)) {
        emit errorOccurred(QString("Cannot open bin file: %1").arg(binPath));
        return;
    }
    for (const FrameData &f : frames) {
        binFile.write(f.data);
    }
    binFile.close();
}

void BatchWriter::writeHeader(const QString headerPath, QString baseName,
                              const QVector<FrameData> &frames,
                              const QVector<LightSensorData> &lightData) {
    QJsonObject root;
    root["version"] = 2;
    root["baseName"] = baseName;
    root["width"] = frames.first().width;
    root["height"] = frames.first().height;
    root["pixelFormat"] = frames.first().pixelFormat;
    root["exposition"] = frames.first().expositionMs;
    root["frameCount"] = frames.size();
    root["endianness"] =
        (QSysInfo::ByteOrder == QSysInfo::LittleEndian) ? "little" : "big";
    root["creationTime"] =
        QDateTime::currentDateTimeUtc().toString(Qt::ISODate);

    QJsonArray framesArray;
    for (const FrameData &f : frames) {
        QJsonObject frameObj;
        frameObj["timestamp_ms"] = f.timestampMs;
        frameObj["bytes"] = f.data.size();
        framesArray.append(frameObj);
    }
    root["frames"] = framesArray;

    if (!lightData.isEmpty()) {
        QJsonArray lightArray;
        for (const LightSensorData &ld : lightData) {
            QJsonObject lightObj;
            lightObj["timestamp"] = ld.dateTime;
            lightObj["integrationTimeMs"] = ld.integrationTimeMs;
            lightObj["gainIndex"] = ld.gainIndex;
            lightObj["sunElevation"] = ld.sunElevation;
            QJsonArray chArr;
            for (quint16 val : ld.channels) chArr.append(val);
            lightObj["channels"] = chArr;
            lightArray.append(lightObj);
        }
        root["lightSensorData"] = lightArray;
    }

    QFile jsonFile(headerPath);
    if (jsonFile.open(QIODevice::WriteOnly)) {
        jsonFile.write(QJsonDocument(root).toJson());
        jsonFile.close();
    } else {
        emit errorOccurred(
            QString("Cannot open json file: %1").arg(headerPath));
    }
}

void BatchWriter::shutdown() { m_shutdown.fetchAndStoreRelease(1); }

QString BatchWriter::generateBaseName(const QString &prefix) const {
    static QAtomicInt seq = 0;
    int seqNum = seq.fetchAndAddRelaxed(1) % 10000;
    QString ts = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    return QString("%1_%2_%3")
        .arg(prefix)
        .arg(ts)
        .arg(seqNum, 4, 10, QChar('0'));
}
