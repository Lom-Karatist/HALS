#include "LightSaver.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMutex>
#include <QtConcurrent/QtConcurrent>

static QMutex mutex;

LightSaver::LightSaver(QObject *parent) : QObject(parent), m_enabled(false) {}

void LightSaver::setSavingPath(const QString &path) {
    m_savingPath = path;
    //    qDebug() << "\t\tLight saving path:" << m_savingPath;
    QDir dir(m_savingPath);
    if (!dir.exists()) dir.mkpath(".");
}

void LightSaver::setEnabled(bool enable) { m_enabled = enable; }
bool LightSaver::isEnabled() const { return m_enabled; }

void LightSaver::saveDataAsync(const LightSensorData &data) {
    //    qDebug() << "\t\tSaving light data with size " <<
    //    data.channels.count();
    if (!m_enabled || m_savingPath.isEmpty()) return;
    (void)QtConcurrent::run(appendToFile, data, m_savingPath);
}

void LightSaver::appendToFile(const LightSensorData &data,
                              const QString &path) {
    QJsonObject obj;
    obj["timestamp"] = data.dateTime;
    obj["integrationTimeMs"] = data.integrationTimeMs;
    obj["gainIndex"] = data.gainIndex;
    obj["sunElevation"] = data.sunElevation;
    QJsonArray arr;
    for (quint16 val : data.channels) arr.append(val);
    obj["channels"] = arr;

    QJsonDocument doc(obj);
    QString fileName = path + "/lightsensor_data.jsonl";
    QFile file(fileName);
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        file.write(doc.toJson() + "\n");
        file.close();
    }
}
