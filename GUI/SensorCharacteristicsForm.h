#ifndef SENSORCHARACTERISTICSFORM_H
#define SENSORCHARACTERISTICSFORM_H

#include <QWidget>

namespace Ui {
class SensorCharacteristicsForm;
}

/**
 * @brief Виджет для отображения характеристик сенсора: GSD, поле зрения,
 * максимальная яркость.
 *
 * Использует три виджета QLCDNumber для отображения числовых значений:
 * - GSD (разрешение на пиксель) в мм/пиксель.
 * - Поле зрения (FOV) в метрах.
 * - Максимальная яркость (0–255 для Mono8, 0–4095 для Mono12).
 *
 * Значения обновляются через публичные слоты setGsd(), setFov(),
 * setMaxBrightness(). Поддерживаются дробные числа (QLCDNumber принимает double
 * и int).
 */
class SensorCharacteristicsForm : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief Конструктор.
     * @param parent Родительский виджет.
     */
    explicit SensorCharacteristicsForm(QWidget *parent = nullptr);
    ~SensorCharacteristicsForm();

public slots:
    /**
     * @brief Установить значение разрешения на пиксель (GSD).
     * @param gsd Значение в мм/пиксель (дробное).
     *
     * Отображается на QLCDNumber с автоматическим округлением до доступного
     * количества цифр (задаётся в .ui).
     */
    void setGsd(double gsd);

    /**
     * @brief Установить значение поля зрения (FOV).
     * @param fov Значение в метрах (дробное).
     */
    void setFov(double fov);

    /**
     * @brief Установить максимальную яркость в кадре.
     * @param max Значение яркости (целое).
     */
    void setMaxBrightness(int max);

private:
    Ui::SensorCharacteristicsForm *ui;  //!< Интерфейс, сгенерированный из .ui.
};

#endif  // SENSORCHARACTERISTICSFORM_H
