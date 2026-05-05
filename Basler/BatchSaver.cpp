#include "BatchSaver.h"

#include <QDateTime>
#include <QMetaObject>

BatchSaver::BatchSaver(const QString &basePath, int maxFramesPerBatch,
                       qint64 maxBufferBytesPerBatch, QObject *parent)
    : QObject(parent),
      m_basePath(basePath),
      m_maxFramesPerBatch(maxFramesPerBatch),
      m_maxBufferBytesPerBatch(maxBufferBytesPerBatch) {
    // Создаём и настраиваем BatchWriter в отдельном потоке
    m_writer = new BatchWriter(basePath, maxFramesPerBatch,
                               maxBufferBytesPerBatch, nullptr);
    m_writer->moveToThread(&m_writerThread);

    connect(m_writer, &BatchWriter::errorOccurred, this,
            &BatchSaver::onWriterError, Qt::QueuedConnection);
    connect(m_writer, &BatchWriter::fileWritten, this,
            &BatchSaver::onWriterFileWritten, Qt::QueuedConnection);
    connect(m_writer, &BatchWriter::flushed, this, &BatchSaver::onWriterFlushed,
            Qt::QueuedConnection);

    m_writerThread.start();

    moveToThread(&m_saverThread);
    m_saverThread.start();

    qRegisterMetaType<BaslerConstants::FrameData>();
}

BatchSaver::~BatchSaver() {
    m_saverThread.quit();
    m_saverThread.wait();

    QMetaObject::invokeMethod(m_writer, "shutdown", Qt::QueuedConnection);
    m_writerThread.quit();
    m_writerThread.wait();
    delete m_writer;
}

void BatchSaver::addFrame(const QString &prefix, int width, int height,
                          int pixelFormat, const QByteArray &data,
                          qint64 timestampMs) {
    Buffer *buf = (prefix == "HS") ? &m_bufferHS : &m_bufferOC;
    if (!buf->active) {
        buf->frames.clear();
        buf->totalBytes = 0;
        buf->width = width;
        buf->height = height;
        buf->pixelFormat = pixelFormat;
        buf->prefix = prefix;
        buf->active = true;
    } else if (buf->width != width || buf->height != height ||
               buf->pixelFormat != pixelFormat) {
        emit errorOccurred(
            QString("Parameter mismatch for %1, discarding frame").arg(prefix));
        return;
    }
    buf->frames.append({data, timestampMs});
    buf->totalBytes += data.size();

    if (buf->frames.size() >= m_maxFramesPerBatch ||
        buf->totalBytes >= m_maxBufferBytesPerBatch) {
        sendBuffer(prefix, *buf);
        buf->active = false;
    }
}

void BatchSaver::flush() {
    // Выполняется в потоке m_saverThread
    if (m_bufferHS.active) {
        sendBuffer("HS", m_bufferHS);
        m_bufferHS.active = false;
    }
    if (m_bufferOC.active) {
        sendBuffer("OC", m_bufferOC);
        m_bufferOC.active = false;
    }
    // Запрашиваем у writer'а завершение (асинхронно)
    QMetaObject::invokeMethod(m_writer, "flush", Qt::QueuedConnection);
}

void BatchSaver::sendBuffer(const QString &prefix, Buffer &buf) {
    if (buf.frames.isEmpty()) return;

    QVector<BaslerConstants::FrameData> frames;
    frames.reserve(buf.frames.size());
    for (const auto &f : buf.frames) {
        frames.append({prefix, buf.width, buf.height, buf.pixelFormat, f.data,
                       f.timestampMs});
    }

    // Отправляем в поток writer'а
    QMetaObject::invokeMethod(
        m_writer, "writeBatch", Qt::QueuedConnection, Q_ARG(QString, prefix),
        Q_ARG(QVector<BaslerConstants::FrameData>, frames));

    buf.frames.clear();
    buf.totalBytes = 0;
}

void BatchSaver::onWriterError(const QString &err) { emit errorOccurred(err); }

void BatchSaver::onWriterFileWritten(const QString &binFile, int count,
                                     const QString &prefix) {
    emit fileWritten(binFile, count, prefix);
}

void BatchSaver::onWriterFlushed() { emit flushed(); }
