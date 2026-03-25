#ifndef HALSWINDOW_H
#define HALSWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class HalsWindow; }
QT_END_NAMESPACE

class HalsWindow : public QMainWindow
{
    Q_OBJECT

public:
    HalsWindow(QWidget *parent = nullptr);
    ~HalsWindow();

private:
    Ui::HalsWindow *ui;
};
#endif // HALSWINDOW_H
