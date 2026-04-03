#ifndef LIGHTSAVER_H
#define LIGHTSAVER_H

#include <QObject>
#include <QString>

#include "LightTypes.h"

/**
 * @brief Класс для асинхронного сохранения данных датчика освещённости в единый
 * JSONL-файл.
 *
 * Каждое измерение дописывается в конец файла lightsensor_data.jsonl в формате
 * JSON Lines (каждая строка – отдельный JSON-объект). Это обеспечивает простой
 * и эффективный способ накопления данных без необходимости перезаписи всего
 * файла.
 *
 * Сохранение выполняется в отдельном потоке с использованием QtConcurrent и
 * защищено мьютексом, что позволяет вызывать метод saveDataAsync() из любого
 * потока без блокировки.
 */
class LightSaver : public QObject {
    Q_OBJECT
public:
    /**
     * @brief Конструктор.
     * @param parent Родительский QObject.
     */
    explicit LightSaver(QObject *parent = nullptr);

    /**
     * @brief Установить путь к каталогу для сохранения файлов.
     * @param path Путь к каталогу (будет создан, если не существует).
     */
    void setSavingPath(const QString &path);

    /**
     * @brief Включить/выключить сохранение данных.
     * @param enable true – сохранять, false – не сохранять.
     */
    void setEnabled(bool enable);

    /**
     * @brief Проверить, включено ли сохранение.
     * @return true, если сохранение активно.
     */
    bool isEnabled() const;

    /**
     * @brief Асинхронно сохранить одно измерение датчика.
     * @param data Структура с данными (временная метка, каналы, параметры).
     *
     * Данные дописываются в файл lightsensor_data.jsonl в каталоге, заданном
     * через setSavingPath(). Вызов не блокирует вызывающий поток.
     */
    void saveDataAsync(const LightSensorData &data);

private:
    /**
     * @brief Статический метод, выполняющий фактическую запись в файл
     * (выполняется в потоке пула).
     * @param data Данные для сохранения.
     * @param path Каталог сохранения.
     */
    static void appendToFile(const LightSensorData &data, const QString &path);

    QString m_savingPath;  //!< Путь к каталогу сохранения.
    bool m_enabled;  //!< Флаг включения сохранения.
};

#endif  // LIGHTSAVER_H
