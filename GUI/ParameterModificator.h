#ifndef PARAMETERMODIFICATOR_H
#define PARAMETERMODIFICATOR_H

#include <QPushButton>
#include <QWidget>

namespace Ui {
class ParameterModificator;
}

class ParameterModificator : public QWidget {
    Q_OBJECT

public:
    explicit ParameterModificator(QWidget *parent = nullptr);
    ~ParameterModificator();

    // Настройка внешнего вида
    void setParameterName(const QString &name, const QString &unit = "");
    void setRange(int min, int max);
    void setSteps(int step1, int step2, int step3);
    void setValue(int value);
    int value() const;

signals:
    void valueChanged(int newValue);

private slots:
    void on_pushButtonMinus_clicked();
    void on_pushButtonMinus2_clicked();
    void on_pushButtonMinus3_clicked();
    void on_pushButtonPlus_clicked();
    void on_pushButtonPlus2_clicked();
    void on_pushButtonPlus3_clicked();
    void on_spinBoxValue_valueChanged(int value);

private:
    void updateButtonTexts();
    void applyButtonStyle(QPushButton *btn, const QColor &color,
                          const QColor &pressedColor);

    Ui::ParameterModificator *ui;
    int m_step1, m_step2, m_step3;
    int m_min, m_max;
};

#endif  // PARAMETERMODIFICATOR_H
