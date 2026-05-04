#ifndef UDPLIGHTSENSORREADER_H
#define UDPLIGHTSENSORREADER_H

#include <QObject>
#include <QUdpSocket>
#include <QThread>
#include "LightTypes.h"

/**
 * @brief Класс для приёма UDP-пакетов от Python-скрипта AS7341.
 *
 * Работает в отдельном потоке. Слушает заданный порт, читает JSON-сообщения,
 * преобразует их в структуру LightSensorData и испускает сигнал dataReceived().
 */
class UdpLightSensorReader : public QObject
{
    Q_OBJECT
public:
    explicit UdpLightSensorReader(int port = 12345, QObject *parent = nullptr);
    ~UdpLightSensorReader();

    void start();
    void stop();

signals:
    void dataReceived(const LightSensorData &data);
    void errorOccurred(const QString &error);

private slots:
    void readPendingDatagrams();

private:
    LightSensorData parseJson(const QByteArray &jsonData) const;

    QUdpSocket *m_udpSocket;
    int m_port;
    bool m_running;
};

#endif // UDPLIGHTSENSORREADER_H