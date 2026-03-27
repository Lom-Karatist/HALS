#ifndef HALSWINDOW_H
#define HALSWINDOW_H

#include <QMainWindow>

#include "StatusIndicator.h"
#include "gps_device.h"

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

    void on_pushButtonSettings_clicked();

    void on_pushButtonSaveParameters_clicked();

    void on_pushButtonStartStop_clicked();

private:
    void setupGui();
    void applyStyleSheet();

    void addStatusIndicators();

    Ui::HalsWindow *ui;
    qreal m_touchStartPos;
};
#endif  // HALSWINDOW_H
