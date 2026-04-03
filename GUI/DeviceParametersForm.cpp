#include "DeviceParametersForm.h"

#include <QGroupBox>
#include <QVBoxLayout>

#include "ui_DeviceParametersForm.h"

DeviceParametersForm::DeviceParametersForm(QWidget *parent)
    : QWidget(parent), ui(new Ui::DeviceParametersForm) {
    ui->setupUi(this);
}

DeviceParametersForm::~DeviceParametersForm() { delete ui; }

void DeviceParametersForm::setDeviceName(const QString &name) {
    ui->groupBox->setTitle(name);
    updateStyleSheet();
}

void DeviceParametersForm::addParameter(const QString &name,
                                        const QString &unit, int min, int max,
                                        int initialValue, int step1, int step2,
                                        int step3) {
    ParameterModificator *param;
    if (m_paramMap.isEmpty()) {
        param = ui->widget;
    } else
        param = ui->widget_2;
    param->setParameterName(name, unit);
    param->setRange(min, max);
    param->setSteps(step1, step2, step3);
    param->setValue(initialValue);
    m_paramMap[param] = name;

    connect(param, &ParameterModificator::valueChanged, this,
            &DeviceParametersForm::onParameterValueChanged);
}

void DeviceParametersForm::addParameter(const ParameterInfo &info) {
    addParameter(info.name, info.unit, info.minVal, info.maxVal,
                 info.initialValue, info.step1, info.step2, info.step3);
}

void DeviceParametersForm::onParameterValueChanged(int value) {
    ParameterModificator *param =
        qobject_cast<ParameterModificator *>(sender());
    if (param && m_paramMap.contains(param)) {
        emit parameterChanged(m_paramMap[param], value);
    }
}

void DeviceParametersForm::updateStyleSheet() {
    setStyleSheet(QString(R"(
                            QGroupBox::title {
                                subcontrol-position: top center;
                                color: #fe9a00;
                            }
                            )"));
}
