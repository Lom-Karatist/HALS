#include "StatusIndicator.h"

#include <QPixmap>

#include "ui_StatusIndicator.h"

StatusIndicator::StatusIndicator(QWidget *parent)
    : QWidget(parent), ui(new Ui::StatusIndicator), m_state(State::Unknown) {
    ui->setupUi(this);
    updateStyle();
}

StatusIndicator::~StatusIndicator() { delete ui; }

void StatusIndicator::setIcon(const QPixmap &icon) {
    ui->labelImage->setPixmap(
        icon.scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void StatusIndicator::setLabelText(const QString &text) {
    ui->labelName->setText(text);
}

void StatusIndicator::setValueText(const QString &text) {
    ui->labelState->setText(text);
}

void StatusIndicator::setState(State state) {
    if (m_state == state) return;
    m_state = state;
    updateStyle();
}

void StatusIndicator::updateStyle() {
    QString borderColor, bgColor, stateColor;

    switch (m_state) {
        case State::Active:
            borderColor = "#00d492";  // зелёный
            bgColor = "rgba(16,185,129,0.1)";
            stateColor = "#00d492";
            break;
        case State::Inactive:
            borderColor = "#ff6467";  // красный
            bgColor = "rgba(239,68,68,0.1)";
            stateColor = "#ff6467";
            break;
        case State::Unknown:
        default:
            borderColor = "#9ca3af";  // серый
            bgColor = "rgba(156,163,175,0.1)";
            stateColor = "#d4d4d8";
            break;
    }

    setStyleSheet(QString("StatusIndicator {"
                          "   border: 2px solid %1;"
                          "   background-color: %2;"
                          "   border-radius: 8px;"
                          "}"
                          // labelName всегда белый
                          "StatusIndicator #labelName {"
                          "   color: #ffffff;"
                          "}"
                          // labelState меняет цвет
                          "StatusIndicator #labelState {"
                          "   color: %3;"
                          "}")
                      .arg(borderColor, bgColor, stateColor));
}
