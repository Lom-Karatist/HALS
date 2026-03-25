#include "HalsWindow.h"
#include "ui_HalsWindow.h"

HalsWindow::HalsWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::HalsWindow)
{
    ui->setupUi(this);

    device.start();

    if (device.isRunning()) {
        qDebug()<<"--------------Writing to file...";
        device.writeFormattedGpsDataToFile(logger::saveFormat::jsonIndented, "D:/HALS/Data/gps.txt");
    }
}

HalsWindow::~HalsWindow()
{
    delete ui;
    if (device.isRunning()) {
        device.stop();
    }
}

