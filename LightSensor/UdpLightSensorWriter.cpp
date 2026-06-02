#include "UdpLightSensorWriter.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>

UdpLightSensorWriter::UdpLightSensorWriter(const QString &address, int port,
                                           QObject *parent)
    : QObject(parent), m_socket(nullptr), m_port(port) {
    m_socket = new QUdpSocket(this);
    m_address = QHostAddress(address);
}

UdpLightSensorWriter::~UdpLightSensorWriter() {
    if (m_socket) m_socket->close();
}

void UdpLightSensorWriter::sendIntegrationTime(int atime, int astep) {
    QJsonObject cmd;
    cmd["type"] = "set_integration_time";
    QJsonObject params;
    params["atime"] = atime;
    params["astep"] = astep;
    cmd["value"] = params;
    sendCommand(QJsonDocument(cmd).toJson(QJsonDocument::Compact));
}

void UdpLightSensorWriter::sendCommand(const QByteArray &jsonData) {
    if (!m_socket) return;
    qint64 bytes = m_socket->writeDatagram(jsonData, m_address, m_port);
    if (bytes == -1)
        qDebug() << "UdpLightSensorWriter: failed to send command:"
                 << m_socket->errorString();
}

void UdpLightSensorWriter::sendGain(double gain) {
    QJsonObject cmd;
    cmd["type"] = "set_gain";
    cmd["value"] = gain;
    sendCommand(QJsonDocument(cmd).toJson(QJsonDocument::Compact));
}

void UdpLightSensorWriter::sendFrameRate(int hz) {
    QJsonObject cmd;
    cmd["type"] = "set_freq";
    cmd["value"] = hz;
    sendCommand(QJsonDocument(cmd).toJson(QJsonDocument::Compact));
}
