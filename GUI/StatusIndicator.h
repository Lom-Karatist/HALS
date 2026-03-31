#ifndef STATUSINDICATOR_H
#define STATUSINDICATOR_H

#include <QWidget>

namespace Ui {
class StatusIndicator;
}

/**
 * @brief Виджет для отображения состояния устройства с иконкой, названием и
 * дополнительным текстом.
 *
 * Виджет имеет три возможных состояния:
 * - Active (зелёный) – устройство активно, готово к работе.
 * - Inactive (красный) – устройство не активно или отключено.
 * - Unknown (серый) – состояние неизвестно, идёт соединение.
 *
 * Для каждого состояния используется своя иконка (суффиксы _active, _inactive,
 * _unknown) и цвет рамки/текста. Виджет реагирует на нажатие мыши, отправляя
 * сигнал clicked(), что позволяет использовать его как интерактивный элемент.
 *
 * Для корректного отображения фона необходимо установить атрибут
 * WA_StyledBackground (выполняется в конструкторе).
 */
class StatusIndicator : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief Перечисление возможных состояний индикатора.
     */
    enum class State {
        Active,  //!< Активно: зелёная рамка, текст, иконка _active.
        Inactive,  //!< Неактивно: красная рамка, текст, иконка _inactive.
        Unknown  //!< Неизвестно: серая рамка, текст "Соединение...", иконка
                 //!< _unknown.
    };
    Q_ENUM(State)

    /**
     * @brief Конструктор.
     * @param parent Родительский виджет.
     * @param baseName Базовое имя файла иконки (без суффикса и расширения).
     *                 Иконки должны лежать в ресурсах и иметь имена:
     *                 baseName + "_active.png", baseName + "_inactive.png",
     *                 baseName + "_unknown.png".
     */
    explicit StatusIndicator(QWidget *parent = nullptr, QString baseName = "");
    ~StatusIndicator();

    /**
     * @brief Установить базовое имя иконки.
     * @param text Новое базовое имя (без суффикса).
     */
    void setIconBaseName(const QString &text);

    /**
     * @brief Установить иконку для текущего состояния.
     * @param nameAppendix Суффикс имени файла (_active, _inactive, _unknown).
     *
     * Обычно вызывается автоматически при смене состояния. Иконка
     * масштабируется до 64×64 пикселей с сохранением пропорций и сглаживанием.
     */
    void setIcon(QString nameAppendix);

    /**
     * @brief Установить текст заголовка (название устройства).
     * @param text Текст, отображаемый слева от иконки.
     */
    void setLabelText(const QString &text);

    /**
     * @brief Установить дополнительный текст состояния (например, "Активен",
     * "Не подключен").
     * @param text Текст, отображаемый справа от иконки.
     */
    void setValueText(const QString &text);

    /**
     * @brief Получить текущее состояние индикатора.
     * @return Текущее состояние (Active/Inactive/Unknown).
     */
    State state() const { return m_state; }

    /**
     * @brief Установить состояние индикатора.
     * @param state Новое состояние.
     *
     * При изменении состояния обновляется цвет рамки, текст состояния (для
     * Unknown) и иконка. Сигнал clicked() при этом не генерируется.
     */
    void setState(State state);

    /**
     * @brief Проверить, активно ли устройство.
     * @return true, если состояние равно State::Active.
     */
    bool isActive() const { return m_state == State::Active; }

signals:
    /**
     * @brief Сигнал, испускаемый при клике на виджет.
     *
     * Позволяет использовать индикатор как кнопку для перехода к настройкам
     * или детальной информации об устройстве.
     */
    void clicked();

protected:
    /**
     * @brief Обработчик нажатия мыши.
     * @param event Событие нажатия.
     *
     * При любом нажатии (левой кнопкой) испускает сигнал clicked().
     */
    void mousePressEvent(QMouseEvent *event) override;

private:
    /**
     * @brief Обновить стиль виджета в соответствии с текущим состоянием.
     *
     * Устанавливает цвет фона, цвет рамки, цвет текста состояния и иконку.
     * Стиль применяется через setStyleSheet.
     */
    void updateStyle();

    Ui::StatusIndicator *ui;  //!< Интерфейс, сгенерированный из .ui.
    State m_state;  //!< Текущее состояние индикатора.
    QString m_iconBaseName;  //!< Базовое имя файла иконки (без суффикса).
};

#endif  // STATUSINDICATOR_H
