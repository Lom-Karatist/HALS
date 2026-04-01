#ifndef HALSWINDOW_H
#define HALSWINDOW_H

#include <QMainWindow>

#include "Components/HalsFacade.h"
#include "OverlayLabel.h"
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

    void updateHsState(bool connectionStatus);
    void updateOcState(bool connectionStatus);

    void updateOcImageLabel(QImage imageToShow, int maxBrightness);
    void updateHsImageLabel(QImage imageToShow, int maxBrightness);

    void updateOcChars(double fovMeters, double gsd);
    void updateHsChars(double fovMeters, double gsd);

    QString formatBytes(qint64 bytes);

    void updateTime();

    void on_pushButtonMakeSnapshot_clicked();

    void setSpectrometerFovRect(const QRect &rect);

private:
    /**
     * @brief Общая настройка проекта.
     */
    void setupProject();

    void initObjects();
    void setupGui();
    void applyStyleSheet();

    void addStatusIndicators();
    void makePageSwitch(QWidget *fromPage, QWidget *toPage);

    Ui::HalsWindow *ui;
    QString m_title;        //!< Заголовок окна.
    QSettings *m_settings;  //!< Объект для работы с настройками приложения.
    qreal m_touchStartPos;

    HalsFacade *m_facade;
    QTimer *m_updatingTimer;

    StatusIndicator *m_usbIndicator;
    StatusIndicator *m_hsIndicator;
    StatusIndicator *m_ocIndicator;
    StatusIndicator *m_sunIndicator;
    StatusIndicator *m_missionIndicator;

    QRect m_hsFovRect;
};
#endif  // HALSWINDOW_H
