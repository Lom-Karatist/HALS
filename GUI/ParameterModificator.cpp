#include "ParameterModificator.h"

#include <QScreen>

#include "ui_ParameterModificator.h"

ParameterModificator::ParameterModificator(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::ParameterModificator),
      m_step1(1),
      m_step2(10),
      m_step3(100),
      m_min(0),
      m_max(100) {
    ui->setupUi(this);
    ui->spinBoxValue->setRange(m_min, m_max);
    ui->spinBoxValue->installEventFilter(this);
    updateButtonTexts();

    applyButtonStyle(ui->pushButtonMinus, QColor(0xff, 0x64, 0x67),
                     QColor(0xcc, 0x50, 0x52));
    applyButtonStyle(ui->pushButtonMinus2, QColor(0xff, 0x64, 0x67),
                     QColor(0xcc, 0x50, 0x52));
    applyButtonStyle(ui->pushButtonMinus3, QColor(0xff, 0x64, 0x67),
                     QColor(0xcc, 0x50, 0x52));
    applyButtonStyle(ui->pushButtonPlus, QColor(0x00, 0xd4, 0x92),
                     QColor(0x00, 0xb0, 0x76));
    applyButtonStyle(ui->pushButtonPlus2, QColor(0x00, 0xd4, 0x92),
                     QColor(0x00, 0xb0, 0x76));
    applyButtonStyle(ui->pushButtonPlus3, QColor(0x00, 0xd4, 0x92),
                     QColor(0x00, 0xb0, 0x76));
}

ParameterModificator::~ParameterModificator() { delete ui; }

void ParameterModificator::setParameterName(const QString &name,
                                            const QString &unit) {
    QString text = name;
    if (!unit.isEmpty()) text += ", " + unit;
    ui->labelParameter->setText(text);
}

void ParameterModificator::setRange(int min, int max) {
    m_min = min;
    m_max = max;
    ui->spinBoxValue->setRange(min, max);
}

void ParameterModificator::setSteps(int step1, int step2, int step3) {
    m_step1 = step1;
    m_step2 = step2;
    m_step3 = step3;
    updateButtonTexts();
}

void ParameterModificator::setValue(int value) {
    ui->spinBoxValue->setValue(value);
}

int ParameterModificator::value() const { return ui->spinBoxValue->value(); }

bool ParameterModificator::eventFilter(QObject *obj, QEvent *event) {
    if (obj == ui->spinBoxValue && event->type() == QEvent::FocusIn) {
        QPoint globalPos = ui->spinBoxValue->mapToGlobal(QPoint(0, 0));
        bool rightAligned =
            (globalPos.x() >
             (QApplication::primaryScreen()->geometry().width() / 2));
        emit requestKeyboard(ui->spinBoxValue, !rightAligned);
        return false;
    }
    return QWidget::eventFilter(obj, event);
}

void ParameterModificator::on_pushButtonMinus_clicked() {
    int newVal = ui->spinBoxValue->value() - m_step1;
    if (newVal >= m_min) ui->spinBoxValue->setValue(newVal);
}

void ParameterModificator::on_pushButtonMinus2_clicked() {
    int newVal = ui->spinBoxValue->value() - m_step2;
    if (newVal >= m_min) ui->spinBoxValue->setValue(newVal);
}

void ParameterModificator::on_pushButtonMinus3_clicked() {
    int newVal = ui->spinBoxValue->value() - m_step3;
    if (newVal >= m_min) ui->spinBoxValue->setValue(newVal);
}

void ParameterModificator::on_pushButtonPlus_clicked() {
    int newVal = ui->spinBoxValue->value() + m_step1;
    if (newVal <= m_max) ui->spinBoxValue->setValue(newVal);
}

void ParameterModificator::on_pushButtonPlus2_clicked() {
    int newVal = ui->spinBoxValue->value() + m_step2;
    if (newVal <= m_max) ui->spinBoxValue->setValue(newVal);
}

void ParameterModificator::on_pushButtonPlus3_clicked() {
    int newVal = ui->spinBoxValue->value() + m_step3;
    if (newVal <= m_max) ui->spinBoxValue->setValue(newVal);
}

void ParameterModificator::on_spinBoxValue_valueChanged(int value) {
    emit valueChanged(value);
}

void ParameterModificator::updateButtonTexts() {
    ui->pushButtonMinus->setText(QString::number(-m_step1));
    ui->pushButtonMinus2->setText(QString::number(-m_step2));
    ui->pushButtonMinus3->setText(QString::number(-m_step3));
    ui->pushButtonPlus->setText("+" + QString::number(m_step1));
    ui->pushButtonPlus2->setText("+" + QString::number(m_step2));
    ui->pushButtonPlus3->setText("+" + QString::number(m_step3));
}

void ParameterModificator::applyButtonStyle(QPushButton *btn,
                                            const QColor &color,
                                            const QColor &pressedColor) {
    btn->setStyleSheet(QString("QPushButton {"
                               "   background-color: rgba(%1, %2, %3, 64);"
                               "   border: 1px solid rgb(%1, %2, %3);"
                               "   border-radius: 0px;"
                               "   color: #ffffff;"
                               "   font-weight: bold;"
                               "   min-height: 33px;"
                               "   min-width: 40px;"
                               "   padding: 0px 4px;"
                               "}"
                               "QPushButton:pressed {"
                               "   background-color: rgba(%4, %5, %6, 128);"
                               "   border: 1px solid rgb(%4, %5, %6);"
                               "}")
                           .arg(color.red())
                           .arg(color.green())
                           .arg(color.blue())
                           .arg(pressedColor.red())
                           .arg(pressedColor.green())
                           .arg(pressedColor.blue()));
}

void ParameterModificator::on_spinBoxValue_editingFinished() {
    emit valueChanged(ui->spinBoxValue->value());
}
