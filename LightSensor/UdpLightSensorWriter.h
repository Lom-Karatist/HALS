#ifndef UDPLIGHTSENSORWRITER_H
#define UDPLIGHTSENSORWRITER_H

#include <QHostAddress>
#include <QObject>
#include <QUdpSocket>

class UdpLightSensorWriter : public QObject {
    Q_OBJECT
public:
    explicit UdpLightSensorWriter(const QString &address = "127.0.0.1",
                                  int port = 12346, QObject *parent = nullptr);
    ~UdpLightSensorWriter();

public slots:
    void sendIntegrationTime(int atime, int astep);
    void sendGain(double gain);  // gain multiplier (0.5 .. 512)
    void sendFrameRate(int hz);

private:
    void sendCommand(const QByteArray &jsonData);
    QUdpSocket *m_socket;
    QHostAddress m_address;
    int m_port;
};

#endif  // UDPLIGHTSENSORWRITER_H
