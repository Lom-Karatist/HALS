#include "StatusIndicator.h"

#include <QPixmap>

#include "ui_StatusIndicator.h"

StatusIndicator::StatusIndicator(QWidget *parent, QString baseName)
    : QWidget(parent),
      ui(new Ui::StatusIndicator),
      m_state(State::Unknown),
      m_iconBaseName(baseName) {
    setAttribute(Qt::WA_StyledBackground, true);
    ui->setupUi(this);
    updateStyle();
}

StatusIndicator::~StatusIndicator() { delete ui; }

void StatusIndicator::setIconBaseName(const QString &text) {
    m_iconBaseName = text;
}

void StatusIndicator::setIcon(QString nameAppendix) {
    ui->labelImage->setPixmap(
        QPixmap(m_iconBaseName + nameAppendix + ".png")
            .scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
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

void StatusIndicator::mousePressEvent(QMouseEvent *event) {
    emit clicked();
    QWidget::mousePressEvent(event);
}

void StatusIndicator::updateStyle() {
    QString bgColor, stateColor, iconNameAppendix;

    switch (m_state) {
        case State::Active:
            bgColor = "rgba(16,185,129,16)";
            stateColor = "#00d492";
            iconNameAppendix = "_active";
            break;
        case State::Inactive:
            bgColor = "rgba(239,68,68,16)";
            stateColor = "#ff6467";
            iconNameAppendix = "_inactive";
            break;
        case State::Unknown:
        default:
            bgColor = "rgba(156,163,175,16)";
            stateColor = "#d4d4d8";
            iconNameAppendix = "_unknown";
            setValueText("Соединение...");
            break;
    }

    setIcon(iconNameAppendix);

    setStyleSheet(QString(R"(
            #StatusIndicator {
                border: 3px solid %2;
                background-color: %1;
                border-radius: 32px;
            }
            #labelImage{
                background: transparent;
            }
            #labelName {
                color: #ffffff;
                background: transparent;
            }
            #labelState {
                color: %2;
                background: transparent;
            }
                        )")
                      .arg(bgColor, stateColor));
}
