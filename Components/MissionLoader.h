#ifndef MISSIONLOADER_H
#define MISSIONLOADER_H

#include "MissionTypes.h"
#include <QString>

class MissionLoader {
public:
    static bool load(const QString &filePath, MissionTask &task);
    static bool save(const QString &filePath, const MissionTask &task);
};

#endif // MISSIONLOADER_H