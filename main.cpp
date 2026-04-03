#include <BaseTools/QrcFilesRestorer.h>

#include <QApplication>

#include "GUI/HalsWindow.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    QrcFilesRestorer::restoreFilesFromQrc(":/Hals/4Release/");
    QrcFilesRestorer::restoreFilesFromQrc(":/LightSensor/4Release/");
    QrcFilesRestorer::restoreFilesFromQrc(":/Basler/4Release/");
    HalsWindow w;
    w.show();
    return a.exec();
}
