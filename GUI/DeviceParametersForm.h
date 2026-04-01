#ifndef DEVICEPARAMETERSFORM_H
#define DEVICEPARAMETERSFORM_H

#include <QMap>
#include <QWidget>

#include "ParameterModificator.h"

namespace Ui {
class DeviceParametersForm;
}

/**
 * @brief Виджет для группировки параметров устройства.
 *
 * Содержит группу параметров (QGroupBox), внутри которой динамически
 * создаются элементы ParameterModificator. Каждый параметр имеет имя,
 * единицы измерения, диапазон значений, шаги изменения.
 *
 * Используется на странице настроек для отображения параметров
 * гиперспектрометра, обзорной камеры, датчика освещённости и эксперимента.
 *
 * @see ParameterModificator
 */
class DeviceParametersForm : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief Конструктор.
     * @param parent Родительский виджет.
     */
    explicit DeviceParametersForm(QWidget *parent = nullptr);
    ~DeviceParametersForm();

    /**
     * @brief Установить название устройства (заголовок группы).
     * @param name Название, отображаемое в QGroupBox.
     */
    void setDeviceName(const QString &name);

    /**
     * @brief Добавить параметр в форму.
     *
     * Создаёт виджет ParameterModificator, настраивает его и добавляет
     * в layout группы. При изменении значения параметра будет испущен
     * сигнал parameterChanged.
     *
     * @param name Отображаемое имя параметра.
     * @param unit Единица измерения (например, "мс", "fps").
     * @param min Минимальное допустимое значение.
     * @param max Максимальное допустимое значение.
     * @param initialValue Начальное значение.
     * @param step1 Первый шаг изменения (малый).
     * @param step2 Второй шаг изменения (средний).
     * @param step3 Третий шаг изменения (большой).
     */
    void addParameter(const QString &name, const QString &unit, int min,
                      int max, int initialValue, int step1 = 1, int step2 = 10,
                      int step3 = 100);

signals:
    /**
     * @brief Сигнал об изменении значения параметра.
     * @param paramName Имя параметра (переданное в addParameter).
     * @param newValue Новое значение.
     */
    void parameterChanged(const QString &paramName, int newValue);

private slots:
    /**
     * @brief Слот, вызываемый при изменении значения в ParameterModificator.
     * @param value Новое значение.
     */
    void onParameterValueChanged(int value);

private:
    /**
     * @brief Обновить стиль виджета (может использоваться для кастомизации).
     */
    void updateStyleSheet();

    Ui::DeviceParametersForm *ui;  //!< Интерфейс, сгенерированный из .ui.
    QMap<ParameterModificator *, QString>
        m_paramMap;  //!< Связь между объектами параметров и их именами.
};

#endif  // DEVICEPARAMETERSFORM_H
