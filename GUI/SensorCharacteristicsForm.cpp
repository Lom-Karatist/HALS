#include "SensorCharacteristicsForm.h"
#include "ui_SensorCharacteristicsForm.h"

SensorCharacteristicsForm::SensorCharacteristicsForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SensorCharacteristicsForm)
{
    ui->setupUi(this);
}

SensorCharacteristicsForm::~SensorCharacteristicsForm()
{
    delete ui;
}
