#include <QApplication>

#include "GUI/HalsWindow.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    HalsWindow w;
    w.show();
    return a.exec();
}
