#include "HalsWindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    HalsWindow w;
    w.show();
    return a.exec();
}
