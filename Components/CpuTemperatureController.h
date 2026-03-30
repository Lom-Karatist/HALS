#ifndef CPUTEMPERATURECONTROLLER_H
#define CPUTEMPERATURECONTROLLER_H

#include <QObject>
#include <QTimer>

class CpuTemperatureController : public QObject {
    Q_OBJECT
public:
    explicit CpuTemperatureController(QObject *parent = nullptr);
    ~CpuTemperatureController();

signals:
    void cpuTemperatureUpdated(QString temperature);

private slots:
    void getCpuTemperature();

private:
    QTimer *m_tempTimer;
};

#endif  // CPUTEMPERATURECONTROLLER_H
