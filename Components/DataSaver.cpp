#include "DataSaver.h"

#include <QDir>

DataSaver::DataSaver(QObject *parent) : QObject{parent} {}

const QString &DataSaver::savingPath() const {
    QReadLocker locker(&m_pathLock);
    return m_savingPath;
}

void DataSaver::setSavingPath(const QString &newSavingPath) {
    QWriteLocker locker(&m_pathLock);
    m_savingPath = newSavingPath;

    QDir dir(newSavingPath);
    if (!dir.exists()) dir.mkpath(".");
    qDebug() << "in data saver" << newSavingPath;
}
