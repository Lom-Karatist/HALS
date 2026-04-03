#ifndef DEVICEPARAMETERSFORM_H
#define DEVICEPARAMETERSFORM_H

#include <QMap>
#include <QWidget>

#include "Components/ParameterTypes.h"
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
    void addParameter(ParameterType type, const QString &name,
                      const QString &unit, int min, int max, int initialValue,
                      int step1 = 1, int step2 = 10, int step3 = 100);

    /**
     * @brief Добавить параметр в форму, используя структуру ParameterInfo.
     * @param info Структура, содержащая все параметры настройки.
     *
     * Перегруженная версия метода addParameter, принимающая структуру
     * ParameterInfo. Позволяет вызывать добавление параметра более компактно,
     * особенно при работе со списками или конфигурационными данными.
     *
     * Пример использования:
     * @code
     * ParameterInfo expInfo("Экспозиция", "мс", 1, 2000, 50, 5, 50, 100);
     * deviceForm->addParameter(expInfo);
     *
     * // Или с использованием конструктора прямо в вызове:
     * deviceForm->addParameter({"Частота", "Гц", 1, 100, 30, 1, 10, 20});
     * @endcode
     */
    void addParameter(const ParameterInfo &info);

    /**
     * @brief Установить значение параметра по его типу.
     * @param type Тип параметра (из перечисления ParameterType).
     * @param value Новое целочисленное значение.
     *
     * Метод используется для синхронизации значений параметров в GUI при их
     * принудительном изменении извне (например, когда камера автоматически
     * корректирует экспозицию из-за ограничений частоты кадров).
     *
     * Если параметр с указанным типом присутствует в форме, его значение
     * обновляется без генерации сигнала parameterChanged (чтобы избежать
     * циклической обратной связи). Виджет ParameterModificator при этом
     * обновляет своё отображение.
     */
    void setParameterValue(ParameterType type, int value);

signals:
    /**
     * @brief Сигнал об изменении значения параметра.
     * @param type Тип параметра (передан в addParameter).
     * @param newValue Новое значение.
     */
    void parameterChanged(ParameterType type, int newValue);

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
    QMap<ParameterType, ParameterModificator *>
        m_paramMap;  //!< Связь между объектами параметров и их типами.
};

#endif  // DEVICEPARAMETERSFORM_H
