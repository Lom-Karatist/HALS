#ifndef HALSWINDOW_H
#define HALSWINDOW_H

#include <QMainWindow>

#include "Components/HalsFacade.h"
#include "StatusIndicator.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class HalsWindow;
}
QT_END_NAMESPACE

class HalsWindow : public QMainWindow {
    Q_OBJECT

public:
    HalsWindow(QWidget *parent = nullptr);
    ~HalsWindow();
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void on_pushButtonQuit_clicked();

    void on_pushButtonUpdateConfiguration_clicked();

    void on_pushButtonStartStop_clicked();

    void setSatellitesCount(const int &satellitesCount);

    void updateCpuTemperature(QString temperature);

    void updateUsbState(bool mounted, qint64 availableBytes, qint64 totalBytes);

    QString formatBytes(qint64 bytes);

    void updateTime();

    void on_pushButtonMakeSnapshot_clicked();

private:
    void initObjects();
    void setupGui();
    void applyStyleSheet();

    void addStatusIndicators();
    void makePageSwitch(QWidget *fromPage, QWidget *toPage);

    Ui::HalsWindow *ui;
    qreal m_touchStartPos;

    HalsFacade *m_facade;
    QTimer *m_updatingTimer;

    StatusIndicator *m_usbIndicator;
    StatusIndicator *m_hsIndicator;
    StatusIndicator *m_ocIndicator;
    StatusIndicator *m_sunIndicator;
    StatusIndicator *m_missionIndicator;
};
#endif  // HALSWINDOW_H
