#ifndef DEVICEPARAMETERSFORM_H
#define DEVICEPARAMETERSFORM_H

#include <QMap>
#include <QWidget>

#include "ParameterModificator.h"

/**
 * @brief Структура для хранения всех параметров, необходимых для создания
 * виджета ParameterModificator.
 *
 * Позволяет компактно передавать в метод addParameter полный набор настроек:
 * - имя и единицу измерения;
 * - диапазон допустимых значений;
 * - начальное значение;
 * - три шага изменения (малый, средний, большой).
 *
 * Использование этой структуры упрощает добавление параметров из
 * конфигурационных файлов, массивов или при массовом создании виджетов.
 */
struct ParameterInfo {
    QString name;  //!< Отображаемое имя параметра.
    QString unit;  //!< Единица измерения (например, "мс", "fps").
    int minVal;  //!< Минимальное допустимое значение.
    int maxVal;  //!< Максимальное допустимое значение.
    int initialValue;  //!< Начальное значение.
    int step1;         //!< Первый шаг изменения (малый).
    int step2;  //!< Второй шаг изменения (средний).
    int step3;  //!< Третий шаг изменения (большой).

    /**
     * @brief Конструктор для удобного создания структуры.
     * @param name_ Имя параметра.
     * @param unit_ Единица измерения.
     * @param min_ Минимум.
     * @param max_ Максимум.
     * @param initialValue_ Начальное значение.
     * @param step1_ Малый шаг.
     * @param step2_ Средний шаг.
     * @param step3_ Большой шаг.
     *
     * Все параметры имеют значения по умолчанию, что позволяет создавать
     * структуру с частичной инициализацией.
     */
    ParameterInfo(const QString &name_ = QString(),
                  const QString &unit_ = QString(), int min_ = 0,
                  int max_ = 100, int initialValue_ = 0, int step1_ = 1,
                  int step2_ = 10, int step3_ = 100)
        : name(name_),
          unit(unit_),
          minVal(min_),
          maxVal(max_),
          initialValue(initialValue_),
          step1(step1_),
          step2(step2_),
          step3(step3_) {}
};

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
