#ifndef HALSWINDOW_H
#define HALSWINDOW_H

#include <QMainWindow>

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

private slots:
    void on_pushButtonQuit_clicked();

private:
    void setupGui();
    void applyStyleSheet();

    Ui::HalsWindow *ui;
};
#endif  // HALSWINDOW_H
