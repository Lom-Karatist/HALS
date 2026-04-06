#ifndef PARAMETERMODIFICATOR_H
#define PARAMETERMODIFICATOR_H

#include <QPushButton>
#include <QSpinBox>
#include <QWidget>

namespace Ui {
class ParameterModificator;
}

/**
 * @brief Виджет для изменения целочисленного параметра с фиксированными шагами.
 *
 * Состоит из:
 * - метки с названием параметра и единицей измерения,
 * - поля QSpinBox без кнопок,
 * - трёх кнопок уменьшения (–) и трёх кнопок увеличения (+), каждая из которых
 *   изменяет значение на заданный шаг (step1, step2, step3).
 *
 * Кнопки имеют цветовое оформление:
 * - уменьшение – красный цвет (неактивное состояние),
 * - увеличение – зелёный цвет (активное состояние),
 * - заливка полупрозрачная, с границей.
 *
 * При изменении значения (вручную через spinbox или кнопками) испускается
 * сигнал valueChanged.
 */
class ParameterModificator : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief Конструктор.
     * @param parent Родительский виджет.
     */
    explicit ParameterModificator(QWidget *parent = nullptr);
    ~ParameterModificator();

    // --- Настройка внешнего вида и поведения ---

    /**
     * @brief Установить имя параметра и единицу измерения.
     * @param name Название параметра (например, "Экспозиция").
     * @param unit Единица измерения (например, "мс"). Если не указана,
     * отображается только имя.
     */
    void setParameterName(const QString &name, const QString &unit = "");

    /**
     * @brief Установить допустимый диапазон значений.
     * @param min Минимум.
     * @param max Максимум.
     */
    void setRange(int min, int max);

    /**
     * @brief Установить шаги изменения.
     * @param step1 Первый шаг (малый).
     * @param step2 Второй шаг (средний).
     * @param step3 Третий шаг (большой).
     */
    void setSteps(int step1, int step2, int step3);

    /**
     * @brief Установить текущее значение.
     * @param value Новое значение.
     */
    void setValue(int value);

    /**
     * @brief Получить текущее значение.
     * @return Текущее значение.
     */
    int value() const;

signals:
    /**
     * @brief Сигнал об изменении значения параметра.
     * @param newValue Новое значение (всегда в пределах [min, max]).
     */
    void valueChanged(int newValue);
    void requestKeyboard(QSpinBox *spinBox, bool rightAligned);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    /**
     * @brief Обработчик нажатия кнопки уменьшения на малый шаг.
     *
     * Уменьшает текущее значение на m_step1, если результат не меньше m_min.
     * Изменение значения в QSpinBox автоматически генерирует сигнал
     * valueChanged.
     */
    void on_pushButtonMinus_clicked();

    /**
     * @brief Обработчик нажатия кнопки уменьшения на средний шаг.
     *
     * Уменьшает текущее значение на m_step2, если результат не меньше m_min.
     */
    void on_pushButtonMinus2_clicked();

    /**
     * @brief Обработчик нажатия кнопки уменьшения на большой шаг.
     *
     * Уменьшает текущее значение на m_step3, если результат не меньше m_min.
     */
    void on_pushButtonMinus3_clicked();

    /**
     * @brief Обработчик нажатия кнопки увеличения на малый шаг.
     *
     * Увеличивает текущее значение на m_step1, если результат не больше m_max.
     */
    void on_pushButtonPlus_clicked();

    /**
     * @brief Обработчик нажатия кнопки увеличения на средний шаг.
     *
     * Увеличивает текущее значение на m_step2, если результат не больше m_max.
     */
    void on_pushButtonPlus2_clicked();

    /**
     * @brief Обработчик нажатия кнопки увеличения на большой шаг.
     *
     * Увеличивает текущее значение на m_step3, если результат не больше m_max.
     */
    void on_pushButtonPlus3_clicked();

    /**
     * @brief Обработчик изменения значения в QSpinBox.
     * @param value Новое значение, установленное пользователем.
     *
     * Этот слот вызывается при вводе числа с клавиатуры или программном
     * изменении значения. Проксирует сигнал наружу, эмитируя
     * valueChanged(value).
     */
    void on_spinBoxValue_valueChanged(int value);

    void on_spinBoxValue_editingFinished();

private:
    /**
     * @brief Обновить тексты кнопок в соответствии с шагами.
     *
     * Устанавливает на кнопках тексты вида "-step", "--step", "---step"
     * и "+step", "++step", "+++step".
     */
    void updateButtonTexts();

    /**
     * @brief Применить единый стиль к кнопке.
     * @param btn Указатель на кнопку.
     * @param color Основной цвет (для границы и фона).
     * @param pressedColor Цвет для состояния pressed.
     *
     * Стиль включает полупрозрачную заливку, границу цвета color,
     * отсутствие скругления и минимальную высоту 48 пикселей.
     */
    void applyButtonStyle(QPushButton *btn, const QColor &color,
                          const QColor &pressedColor);

    Ui::ParameterModificator *ui;  //!< Интерфейс, сгенерированный из .ui.
    int m_step1, m_step2, m_step3;  //!< Три шага изменения значения.
    int m_min, m_max;               //!< Минимум и максимум.
};

#endif  // PARAMETERMODIFICATOR_H
