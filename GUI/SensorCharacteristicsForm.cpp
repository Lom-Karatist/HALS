#include "SensorCharacteristicsForm.h"

#include "ui_SensorCharacteristicsForm.h"

SensorCharacteristicsForm::SensorCharacteristicsForm(QWidget *parent)
    : QWidget(parent), ui(new Ui::SensorCharacteristicsForm) {
    ui->setupUi(this);
}

SensorCharacteristicsForm::~SensorCharacteristicsForm() { delete ui; }

void SensorCharacteristicsForm::setGsd(double gsd) {
    ui->lcdNumberResolution->display(gsd);
}

void SensorCharacteristicsForm::setFov(double fov) {
    ui->lcdNumberFov->display(fov);
}

void SensorCharacteristicsForm::setMaxBrightness(int max) {
    ui->lcdNumberMaximum->display(max);
}
