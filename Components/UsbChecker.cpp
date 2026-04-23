#include "UsbChecker.h"

#include <QDebug>
#include <QStorageInfo>

UsbChecker::UsbChecker(QObject *parent)
    : QObject(parent),
      m_lastMounted(false),
      m_lastAvailable(0),
      m_lastTotal(0),
      m_timer(nullptr) {}

UsbChecker::~UsbChecker() {
    stopMonitoring();
    if (m_timer) delete m_timer;
}

void UsbChecker::startMonitoring(int intervalMs) {
    if (!m_timer) {
        m_timer = new QTimer(this);
        connect(m_timer, &QTimer::timeout, this, &UsbChecker::onTimer);
    }
    m_timer->start(intervalMs);
}

void UsbChecker::stopMonitoring() {
    if (m_timer) m_timer->stop();
}

void UsbChecker::check() {
    bool found = false;
    QString mountPath;
    qint64 available = 0;
    qint64 total = 0;

    QList<QStorageInfo> storages = QStorageInfo::mountedVolumes();
    for (const QStorageInfo &storage : storages) {
        if (!storage.isValid() || !storage.isReady()) continue;

        QString root = storage.rootPath();
        QString fsType = storage.fileSystemType();

#ifdef Q_OS_WIN
        if (root != "C:/" && root != "D:/" && root != "E:/" && root != "F:/" &&
            root != "G:/" && root != "S:/" && fsType != "CDFS") {
            mountPath = root;
            available = storage.bytesAvailable();
            total = storage.bytesTotal();
            found = true;
            break;
        }
#else
        // Linux: по пути монтирования или типу ФС
        if (root == "/" || root == "/boot" || root.startsWith("/etc") ||
            root.startsWith("/var")) {
            continue;
        }
        if (root.startsWith("/media/") || root.startsWith("/mnt/") ||
            fsType == "vfat" || fsType == "exfat" || fsType == "fuseblk") {
            mountPath = root;
            available = storage.bytesAvailable();
            total = storage.bytesTotal();
            found = true;
            break;
        }

#endif
    }

    if (found != m_lastMounted ||
        (found && (mountPath != m_lastPath || available != m_lastAvailable ||
                   total != m_lastTotal))) {
        m_lastMounted = found;
        if (found) {
            m_lastPath = mountPath;
            m_lastAvailable = available;
            m_lastTotal = total;
        } else {
            m_lastPath.clear();
            m_lastAvailable = 0;
            m_lastTotal = 0;
        }
        emit usbStatusChanged(found, m_lastAvailable, m_lastTotal);
    } else {
        if (!found) emit usbStatusChanged(found, m_lastAvailable, m_lastTotal);
    }
}

const QString &UsbChecker::lastPath() const { return m_lastPath; }

void UsbChecker::onTimer() {
    check();
    if (m_lastMounted) {
        emit usbSpaceUpdated(m_lastAvailable, m_lastTotal);
    }
}
