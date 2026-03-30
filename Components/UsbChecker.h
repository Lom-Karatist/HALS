#ifndef USBCHECKER_H
#define USBCHECKER_H

#include <QObject>
#include <QStorageInfo>
#include <QTimer>

class UsbChecker : public QObject {
    Q_OBJECT
public:
    explicit UsbChecker(QObject *parent = nullptr);
    ~UsbChecker();

    void check();

signals:
    void usbStatusChanged(bool mounted, qint64 availableBytes,
                          qint64 totalBytes);

private slots:

private:
    bool m_lastMounted;
    QString m_lastPath;
    qint64 m_lastAvailable;
    qint64 m_lastTotal;
};

#endif  // USBCHECKER_H
