#ifndef USBCHECKER_H
#define USBCHECKER_H

#include <QObject>
#include <QStorageInfo>

/**
 * @brief Класс для периодической проверки подключения USB-накопителей.
 *
 * Использует QStorageInfo для определения смонтированных съёмных устройств.
 * Эмитирует сигнал usbStatusChanged при изменении состояния USB-накопителя.
 * Поддерживает Windows (исключает системные диски) и Linux (проверяет пути
 * /media/, /mnt/ и типы ФС vfat/exfat/fuseblk).
 *
 * @note Объект предназначен для использования в отдельном потоке.
 *       Для периодической проверки можно использовать таймер или вызывать
 *       check() вручную.
 */
class UsbChecker : public QObject {
    Q_OBJECT
public:
    /**
     * @brief Конструктор.
     * @param parent Родительский QObject.
     */
    explicit UsbChecker(QObject *parent = nullptr);
    ~UsbChecker();

    /**
     * @brief Выполнить проверку подключения USB-накопителей.
     *
     * Сканирует все смонтированные тома, определяет подходящий съёмный
     * накопитель (по критериям платформы). При изменении состояния
     * (подключение/отключение или изменение доступного пространства)
     * эмитирует сигнал usbStatusChanged.
     */
    void check();

signals:
    /**
     * @brief Сигнал об изменении состояния USB-накопителя.
     * @param mounted true, если найден подходящий USB-накопитель.
     * @param availableBytes Доступное пространство в байтах (если mounted ==
     * true).
     * @param totalBytes Общее пространство в байтах (если mounted == true).
     */
    void usbStatusChanged(bool mounted, qint64 availableBytes,
                          qint64 totalBytes);

private:
    bool m_lastMounted;  //!< Предыдущее состояние наличия USB.
    QString m_lastPath;  //!< Путь к последнему найденному USB.
    qint64 m_lastAvailable;  //!< Предыдущее доступное пространство.
    qint64 m_lastTotal;  //!< Предыдущее общее пространство.
};

#endif  // USBCHECKER_H
