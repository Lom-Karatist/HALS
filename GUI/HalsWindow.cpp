#include "HalsWindow.h"
#include "ui_HalsWindow.h"

HalsWindow::HalsWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::HalsWindow)
{
    ui->setupUi(this);
}

HalsWindow::~HalsWindow()
{
    delete ui;
}

