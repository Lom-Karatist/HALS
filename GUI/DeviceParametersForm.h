#ifndef DEVICEPARAMETERSFORM_H
#define DEVICEPARAMETERSFORM_H

#include <QMap>
#include <QWidget>

#include "ParameterModificator.h"

namespace Ui {
class DeviceParametersForm;
}

class DeviceParametersForm : public QWidget {
    Q_OBJECT

public:
    explicit DeviceParametersForm(QWidget *parent = nullptr);
    ~DeviceParametersForm();

    void setDeviceName(const QString &name);
    void addParameter(const QString &name, const QString &unit, int min,
                      int max, int initialValue, int step1 = 1, int step2 = 10,
                      int step3 = 100);

signals:
    void parameterChanged(const QString &paramName, int newValue);

private slots:
    void onParameterValueChanged(int value);

private:
    void updateStyleSheet();

    Ui::DeviceParametersForm *ui;
    QMap<ParameterModificator *, QString> m_paramMap;
};

#endif  // DEVICEPARAMETERSFORM_H
