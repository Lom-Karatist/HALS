#include "UdpLightSensorReader.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

UdpLightSensorReader::UdpLightSensorReader(int port, QObject *parent)
    : QObject(parent), m_udpSocket(nullptr), m_port(port), m_running(false) {}

UdpLightSensorReader::~UdpLightSensorReader() { stop(); }

void UdpLightSensorReader::start() {
    if (m_running) return;

    m_udpSocket = new QUdpSocket(this);
    if (!m_udpSocket->bind(QHostAddress::LocalHost, m_port)) {
        emit errorOccurred(QString("UDP bind failed on port %1").arg(m_port));
        delete m_udpSocket;
        m_udpSocket = nullptr;
        return;
    }

    connect(m_udpSocket, &QUdpSocket::readyRead, this,
            &UdpLightSensorReader::readPendingDatagrams);
    m_running = true;
    qDebug() << "UdpLightSensorReader started on port" << m_port;
}

void UdpLightSensorReader::stop() {
    if (!m_running) return;
    m_running = false;
    if (m_udpSocket) {
        m_udpSocket->close();
        delete m_udpSocket;
        m_udpSocket = nullptr;
    }
    qDebug() << "UdpLightSensorReader stopped";
}

void UdpLightSensorReader::readPendingDatagrams() {
    while (m_udpSocket && m_udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(m_udpSocket->pendingDatagramSize());
        m_udpSocket->readDatagram(datagram.data(), datagram.size());

        LightSensorData data = parseJson(datagram);
        if (!data.channels.isEmpty()) {
            emit dataReceived(data);
        } else {
            qDebug() << "Failed to parse UDP datagram:" << datagram;
        }
    }
}

LightSensorData UdpLightSensorReader::parseJson(
    const QByteArray &jsonData) const {
    LightSensorData data;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    if (!doc.isObject()) return data;

    QJsonObject obj = doc.object();
    data.dateTime = obj["timestamp"].toString();
    data.integrationTimeMs = obj["integration_ms"].toInt();
    data.gainIndex =
        obj["gain"].toInt();  // усиление в виде индекса (например 32)
    data.sunElevation = 0.0;  // пока не передаётся, можно будет добавить позже

    // Каналы: ожидаем 10 значений: 415,445,480,515,555,590,630,680,912,clear
    QVector<quint16> ch(11, 0);
    ch[LightChannel::CH_F1] = obj["415nm"].toInt();
    ch[LightChannel::CH_F2] = obj["445nm"].toInt();
    ch[LightChannel::CH_F3] = obj["480nm"].toInt();
    ch[LightChannel::CH_F4] = obj["515nm"].toInt();
    ch[LightChannel::CH_F5] = obj["555nm"].toInt();
    ch[LightChannel::CH_F6] = obj["590nm"].toInt();
    ch[LightChannel::CH_F7] = obj["630nm"].toInt();
    ch[LightChannel::CH_F8] = obj["680nm"].toInt();
    ch[LightChannel::CH_NIR] = obj["912nm"].toInt();
    ch[LightChannel::CH_CLEAR] = obj["clear"].toInt();
    // CH_FD – пока не используется
    data.channels = ch;

    return data;
}
