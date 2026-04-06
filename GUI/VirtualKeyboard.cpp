#include "VirtualKeyboard.h"

#include <QGridLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QMainWindow>

VirtualKeyboard::VirtualKeyboard(QWidget *parent)
    : QWidget(parent), m_target(nullptr), m_rightAligned(true) {
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(
        "VirtualKeyboard {border: 1px solid #fe9a00; border-radius: 15px;}"
        "background-color: #1e1e1e; border-radius: 15px;"
        "QPushButton { min-height: 40px; background-color: #3f3f46; color: "
        "white; border-radius: 8px; font-size: 22px; }"
        "QPushButton:pressed { background-color: #52525b; }");
    setFixedSize(320, 350);
    createButtons();
    layoutButtons();
    hide();
}

void VirtualKeyboard::setTarget(QSpinBox *spinBox, bool rightAligned) {
    m_target = spinBox;
    m_rightAligned = rightAligned;
    if (m_target) {
        m_currentInput = QString::number(m_target->value());
        updateDisplay();
    }
    updatePosition();
}

void VirtualKeyboard::show() {
    if (!m_target) return;
    updatePosition();
    QWidget::show();
    raise();
    activateWindow();
}

void VirtualKeyboard::updatePosition() {
    if (!parentWidget()) return;
    QRect parentRect = parentWidget()->rect();
    int x = m_rightAligned ? parentRect.width() - width() - 10 : 10;
    move(x, 5);
}

void VirtualKeyboard::createButtons() {
    // Цифровые кнопки 0-9
    for (int i = 0; i <= 9; ++i) {
        QPushButton *btn = new QPushButton(QString::number(i), this);
        btn->setStyleSheet(
            "QPushButton { background-color: #3f3f46; color: white; "
            "border-radius: 8px; font-size: 22px; }"
            "QPushButton:pressed { background-color: #52525b; }");
        connect(btn, &QPushButton::clicked, this,
                &VirtualKeyboard::onDigitClicked);
        m_digitButtons.append(btn);
    }
    // Кнопка Backspace (⌫)
    m_backspaceBtn = new QPushButton("⌫", this);
    m_backspaceBtn->setStyleSheet(
        "QPushButton { background-color: #ff6467; color: white; border-radius: "
        "8px; font-size: 22px; }"
        "QPushButton:pressed { background-color: #cc5052; }");
    connect(m_backspaceBtn, &QPushButton::clicked, this,
            &VirtualKeyboard::onBackspace);

    // Кнопка Enter
    m_enterBtn = new QPushButton("Enter", this);
    m_enterBtn->setStyleSheet(
        "QPushButton { background-color: #00d492; color: white; border-radius: "
        "8px; font-size: 20px; }"
        "QPushButton:pressed { background-color: #00b076; }");
    connect(m_enterBtn, &QPushButton::clicked, this, &VirtualKeyboard::onEnter);

    // Кнопка Cancel
    m_cancelBtn = new QPushButton("Cancel", this);
    m_cancelBtn->setStyleSheet(
        "QPushButton { background-color: #6b7280; color: white; border-radius: "
        "8px; font-size: 20px; }"
        "QPushButton:pressed { background-color: #4b5563; }");
    connect(m_cancelBtn, &QPushButton::clicked, this,
            &VirtualKeyboard::onCancel);

    // Дисплей
    m_display = new QLabel(this);
    m_display->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_display->setStyleSheet(
        "background-color: #1f1f1f; color: #fe9a00; font-size: 28px; padding: "
        "8px; border-radius: 8px;");
    m_display->setFixedHeight(40);
}

void VirtualKeyboard::layoutButtons() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->addWidget(m_display);

    QGridLayout *gridLayout = new QGridLayout();
    gridLayout->setSpacing(10);
    for (int i = 0; i < 3; ++i) gridLayout->setColumnStretch(i, 1);

    int idx = 1;  // цифра 1
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            gridLayout->addWidget(m_digitButtons[idx++], row, col);
        }
    }

    gridLayout->addWidget(m_digitButtons[0], 3, 1);
    gridLayout->addWidget(m_backspaceBtn, 3, 2);
    QWidget *empty = new QWidget(this);
    empty->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    gridLayout->addWidget(empty, 3, 0);

    mainLayout->addLayout(gridLayout);

    // Горизонтальный слой для Enter и Cancel
    QHBoxLayout *actionLayout = new QHBoxLayout();
    actionLayout->setSpacing(10);
    actionLayout->addWidget(m_enterBtn);
    actionLayout->addWidget(m_cancelBtn);
    actionLayout->setStretch(0, 1);
    actionLayout->setStretch(1, 1);
    mainLayout->addLayout(actionLayout);
}

void VirtualKeyboard::onDigitClicked() {
    QPushButton *btn = qobject_cast<QPushButton *>(sender());
    if (btn) {
        int digit = btn->text().toInt();
        appendDigit(digit);
    }
}

void VirtualKeyboard::appendDigit(int digit) {
    m_currentInput.append(QString::number(digit));
    updateDisplay();
}

void VirtualKeyboard::updateDisplay() { m_display->setText(m_currentInput); }

void VirtualKeyboard::onBackspace() {
    if (!m_currentInput.isEmpty()) m_currentInput.chop(1);
    updateDisplay();
}

void VirtualKeyboard::onEnter() {
    if (m_target) {
        bool ok;
        int val = m_currentInput.toInt(&ok);
        if (ok) m_target->setValue(val);
    }
    hide();
}

void VirtualKeyboard::onCancel() { hide(); }

void VirtualKeyboard::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        onEnter();
    } else if (event->key() == Qt::Key_Escape) {
        onCancel();
    } else if (event->key() == Qt::Key_Backspace) {
        onBackspace();
    } else {
        int digit = event->text().toInt();
        if (digit >= 0 && digit <= 9) appendDigit(digit);
    }
}
