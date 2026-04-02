#ifndef DATASAVER_H
#define DATASAVER_H

#include <QObject>
#include <QReadWriteLock>

class DataSaver : public QObject {
    Q_OBJECT
public:
    explicit DataSaver(QObject *parent = nullptr);

    const QString &savingPath() const;
    void setSavingPath(const QString &newSavingPath);

signals:

private:
    mutable QReadWriteLock m_pathLock;  // защита m_savingPath
    QString m_savingPath;
};

#endif  // DATASAVER_H
