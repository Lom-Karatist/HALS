#ifndef LIGHTSETTINGS_H
#define LIGHTSETTINGS_H

#include <QObject>
#include <QSettings>

/**
 * @brief Загрузка и сохранение настроек датчика освещённости (LS.ini).
 */
class LightSettings : public QObject {
    Q_OBJECT
public:
    explicit LightSettings(QObject *parent = nullptr);

    void loadFromFile();
    void saveToFile() const;

    // Параметры датчика
    int integrationTimeMs;  // время интеграции в миллисекундах (1..1000)
    int gainIndex;          // индекс усиления (0..10, см. преобразование в значение AGAIN)
    int frameRateHz;        // целевая частота опроса в Гц (1..60)

private:
    QSettings m_settings;
};

#endif // LIGHTSETTINGS_H