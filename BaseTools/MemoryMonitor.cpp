#include "MemoryMonitor.h"
#include <QDebug>

#ifdef Q_OS_LINUX
#include <QFile>
#include <QTextStream>
#endif

#ifdef Q_OS_WIN
#include <windows.h>
#endif

MemoryMonitor::MemoryMonitor(QObject *parent)
    : QObject(parent), m_timer(nullptr) {}

MemoryMonitor::~MemoryMonitor() {
    stopMonitoring();
    if (m_timer) delete m_timer;
}

void MemoryMonitor::startMonitoring(int intervalMs) {
    if (!m_timer) {
        m_timer = new QTimer(this);
        connect(m_timer, &QTimer::timeout, this, &MemoryMonitor::onTimer);
    }
    m_timer->start(intervalMs);
}

void MemoryMonitor::stopMonitoring() {
    if (m_timer) m_timer->stop();
}

void MemoryMonitor::check() {
    qint64 total = 0, available = 0;
    if (readSystemMemory(total, available)) {
        emit memoryUsageUpdated(total, available, total - available);
    } else {
        qWarning() << "MemoryMonitor: failed to read system memory";
    }
}

void MemoryMonitor::onTimer() {
    check();
}

bool MemoryMonitor::readSystemMemory(qint64 &totalBytes, qint64 &availableBytes) {
#ifdef Q_OS_LINUX
    QFile file("/proc/meminfo");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    QTextStream stream(&file);
    QString line;
    qint64 memTotal = 0, memAvailable = 0;
    while (stream.readLineInto(&line)) {
        if (line.startsWith("MemTotal:")) {
            memTotal = line.split(" ", Qt::SkipEmptyParts)[1].toLongLong() * 1024;
        } else if (line.startsWith("MemAvailable:")) {
            memAvailable = line.split(" ", Qt::SkipEmptyParts)[1].toLongLong() * 1024;
            break;
        }
    }
    if (memTotal == 0 || memAvailable == 0) {
        return false;
    }
    totalBytes = memTotal;
    availableBytes = memAvailable;
    return true;
#elif defined(Q_OS_WIN)
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    if (!GlobalMemoryStatusEx(&statex)) {
        return false;
    }
    totalBytes = statex.ullTotalPhys;
    availableBytes = statex.ullAvailPhys;
    return true;
#else
    Q_UNUSED(totalBytes);
    Q_UNUSED(availableBytes);
    return false;
#endif
}