#ifndef SENSORCHARACTERISTICSFORM_H
#define SENSORCHARACTERISTICSFORM_H

#include <QWidget>

namespace Ui {
class SensorCharacteristicsForm;
}

class SensorCharacteristicsForm : public QWidget
{
    Q_OBJECT

public:
    explicit SensorCharacteristicsForm(QWidget *parent = nullptr);
    ~SensorCharacteristicsForm();

private:
    Ui::SensorCharacteristicsForm *ui;
};

#endif // SENSORCHARACTERISTICSFORM_H
