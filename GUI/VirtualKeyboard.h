// VirtualKeyboard.h
#ifndef VIRTUALKEYBOARD_H
#define VIRTUALKEYBOARD_H

#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QWidget>

/**
 * @brief Виртуальная цифровая клавиатура для ввода целых чисел.
 *
 * Появляется при фокусе на QSpinBox и позволяет вводить цифры 0-9,
 * удалять последний символ, подтверждать ввод (Enter) или отменять (Cancel).
 * Клавиатура позиционируется справа или слева от экрана в зависимости от
 * положения целевого spinBox.
 */
class VirtualKeyboard : public QWidget {
    Q_OBJECT
public:
    explicit VirtualKeyboard(QWidget *parent = nullptr);

    /**
     * @brief Привязать клавиатуру к определённому spinBox и указать сторону.
     * @param spinBox Целевой виджет.
     * @param rightAligned true – клавиатура справа, false – слева.
     */
    void setTarget(QSpinBox *spinBox, bool rightAligned);
    void show();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onDigitClicked();
    void onBackspace();
    void onEnter();
    void onCancel();

private:
    void createButtons();
    void layoutButtons();
    void appendDigit(int digit);
    void updateDisplay();
    void updatePosition();

    QSpinBox *m_target;
    QString m_currentInput;
    QList<QPushButton *> m_digitButtons;
    QPushButton *m_backspaceBtn, *m_enterBtn, *m_cancelBtn;
    QLabel *m_display;
    bool m_rightAligned;
};

#endif  // VIRTUALKEYBOARD_H
