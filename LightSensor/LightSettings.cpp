#include "LightSettings.h"

#include <QDebug>

LightSettings::LightSettings(QObject *parent, QString fileName)
    : QObject(parent) {
    qDebug() << "settings constructor" << fileName;
    m_settings = new QSettings(fileName, QSettings::IniFormat);
    loadFromFile();
}

LightSettings::~LightSettings() { delete m_settings; }

void LightSettings::loadFromFile() {
    m_integrationTimeMs = m_settings->value("IntegrationTimeMs", 50).toInt();
    m_gainIndex = m_settings->value("GainIndex", 6).toInt();  // 6 -> 64x
    m_frameRateHz = m_settings->value("FrameRateHz", 10).toInt();

    // Ограничения
    if (m_integrationTimeMs < 1) m_integrationTimeMs = 1;
    if (m_integrationTimeMs > 1000) m_integrationTimeMs = 1000;
    if (m_gainIndex < 0) m_gainIndex = 0;
    if (m_gainIndex > 10) m_gainIndex = 10;
    if (m_frameRateHz < 1) m_frameRateHz = 1;
    if (m_frameRateHz > 60) m_frameRateHz = 60;
}

void LightSettings::saveToFile() const {
    m_settings->setValue("IntegrationTimeMs", m_integrationTimeMs);
    m_settings->setValue("GainIndex", m_gainIndex);
    m_settings->setValue("FrameRateHz", m_frameRateHz);
    m_settings->sync();
}

int LightSettings::integrationTimeMs() const { return m_integrationTimeMs; }

void LightSettings::setIntegrationTimeMs(int newIntegrationTimeMs) {
    m_integrationTimeMs = newIntegrationTimeMs;
    saveToFile();
}

int LightSettings::gainIndex() const { return m_gainIndex; }

void LightSettings::setGainIndex(int newGainIndex) {
    m_gainIndex = newGainIndex;
    saveToFile();
}

int LightSettings::frameRateHz() const { return m_frameRateHz; }

void LightSettings::setFrameRateHz(int newFrameRateHz) {
    m_frameRateHz = newFrameRateHz;
    saveToFile();
}
