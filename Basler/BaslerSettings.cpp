#include "BaslerSettings.h"

#include <QFile>

BaslerSettings::BaslerSettings(QObject *parent, QString fileName)
    : QObject{parent} {
    m_settings = createSettingsObject(fileName);
}

BaslerSettings::~BaslerSettings() { delete m_settings; }

BaslerCameraParams BaslerSettings::loadParamsFromFile(bool isMaster) {
    BaslerCameraParams params;
    BaslerCameraParams defaultParams = loadDefaultParamsFromFile(isMaster);

    m_settings->beginGroup("Camera");
    params.serialNumber = m_settings->value("serialNumber", "").toString();
    params.isMaster = m_settings->value("isMaster", isMaster).toBool();

    params.exposureTime =
        m_settings->value("exposureTime", defaultParams.exposureTime)
            .toDouble();
    params.gain = m_settings->value("gain", defaultParams.gain).toDouble();
    params.acquisitionFrameRate =
        m_settings
            ->value("acquisitionFrameRate", defaultParams.acquisitionFrameRate)
            .toDouble();

    QString defaultPixFmt;
    switch (defaultParams.pixelFormat) {
        case PixelType_Mono8:
            defaultPixFmt = BaslerConstants::pixelFormats().at(0);
            break;
        case PixelType_Mono12:
            defaultPixFmt = BaslerConstants::pixelFormats().at(1);
            break;
        case PixelType_Mono12p:
            defaultPixFmt = BaslerConstants::pixelFormats().at(2);
            break;
    }
    QString pixFmt = m_settings->value("pixelFormat", defaultPixFmt).toString();
    if (pixFmt == BaslerConstants::pixelFormats().at(0))
        params.pixelFormat = PixelType_Mono8;
    else if (pixFmt == BaslerConstants::pixelFormats().at(1))
        params.pixelFormat = PixelType_Mono12;
    else if (pixFmt == BaslerConstants::pixelFormats().at(2))
        params.pixelFormat = PixelType_Mono12p;

    params.width = m_settings->value("width", defaultParams.width).toInt();
    params.height = m_settings->value("height", defaultParams.height).toInt();
    params.offsetX =
        m_settings->value("offsetX", defaultParams.offsetX).toInt();
    params.offsetY =
        m_settings->value("offsetY", defaultParams.offsetY).toInt();
    params.binningHorizontal =
        m_settings->value("binningHorizontal", defaultParams.binningHorizontal)
            .toInt();
    params.binningVertical =
        m_settings->value("binningVertical", defaultParams.binningVertical)
            .toInt();

    QString binHMode =
        m_settings->value("binningHorizontalMode", "Sum").toString();
    if (binHMode == BaslerConstants::binningModes().at(1))
        params.binningHorizontalMode = BinningHorizontalMode_Average;
    else if (binHMode == BaslerConstants::binningModes().at(0))
        params.binningHorizontalMode = BinningHorizontalMode_Sum;

    QString binVMode =
        m_settings->value("binningVerticalMode", "Sum").toString();
    if (binVMode == BaslerConstants::binningModes().at(1))
        params.binningVerticalMode = BinningVerticalMode_Average;
    else if (binVMode == BaslerConstants::binningModes().at(0))
        params.binningVerticalMode = BinningVerticalMode_Sum;

    m_settings->endGroup();

    return params;
}

BaslerCameraParams BaslerSettings::loadDefaultParamsFromFile(bool isMaster) {
    QString resourcePath;
    if (isMaster)
        resourcePath = ":/Basler/4Release/HS.ini";
    else {
        resourcePath = ":/Basler/4Release/OC.ini";
    }
    QSettings resourceSettings = QSettings(resourcePath, QSettings::IniFormat);

    BaslerCameraParams params;
    resourceSettings.beginGroup("Camera");

    params.serialNumber = resourceSettings.value("serialNumber", "").toString();
    params.isMaster = isMaster;
    params.exposureTime =
        resourceSettings.value("exposureTime", 50000.0).toDouble();  // 50 мс
    params.gain = resourceSettings.value("gain", 0.0).toDouble();
    params.acquisitionFrameRate =
        resourceSettings.value("acquisitionFrameRate", 20.0).toDouble();

    QString pixFmt =
        resourceSettings.value("pixelFormat", "Mono12p").toString();
    if (pixFmt == "Mono8")
        params.pixelFormat = PixelType_Mono8;
    else if (pixFmt == "Mono12")
        params.pixelFormat = PixelType_Mono12;
    else if (pixFmt == "Mono12p")
        params.pixelFormat = PixelType_Mono12p;
    else
        params.pixelFormat = PixelType_Mono12p;

    params.width = resourceSettings.value("width", 1920).toInt();
    params.height = resourceSettings.value("height", 1200).toInt();
    params.offsetX = resourceSettings.value("offsetX", 0).toInt();
    params.offsetY = resourceSettings.value("offsetY", 0).toInt();
    params.binningHorizontal =
        resourceSettings.value("binningHorizontal", 1).toInt();
    params.binningVertical =
        resourceSettings.value("binningVertical", 1).toInt();

    QString binHMode =
        resourceSettings.value("binningHorizontalMode", "Sum").toString();
    params.binningHorizontalMode = (binHMode == "Average")
                                       ? BinningHorizontalMode_Average
                                       : BinningHorizontalMode_Sum;

    QString binVMode =
        resourceSettings.value("binningVerticalMode", "Sum").toString();
    params.binningVerticalMode = (binVMode == "Average")
                                     ? BinningVerticalMode_Average
                                     : BinningVerticalMode_Sum;

    resourceSettings.endGroup();
    qDebug() << "Def params were loaded:" << params.isMaster << params.offsetX;
    return params;
}

void BaslerSettings::saveParams(const BaslerCameraParams &cameraParams) {
    m_settings->beginGroup("Camera");

    m_settings->setValue("serialNumber", cameraParams.serialNumber);
    m_settings->setValue("isMaster", cameraParams.isMaster);
    m_settings->setValue("exposureTime", cameraParams.exposureTime);
    m_settings->setValue("gain", cameraParams.gain);
    m_settings->setValue("acquisitionFrameRate",
                         cameraParams.acquisitionFrameRate);

    QString pixFmt;
    if (cameraParams.pixelFormat == PixelType_Mono8)
        pixFmt = BaslerConstants::pixelFormats().at(0);
    else if (cameraParams.pixelFormat == PixelType_Mono12)
        pixFmt = BaslerConstants::pixelFormats().at(1);
    else if (cameraParams.pixelFormat == PixelType_Mono12p)
        pixFmt = BaslerConstants::pixelFormats().at(2);
    else
        pixFmt =
            BaslerConstants::pixelFormats().at(0);  // значение по умолчанию
    m_settings->setValue("pixelFormat", pixFmt);

    m_settings->setValue("width", cameraParams.width);
    m_settings->setValue("height", cameraParams.height);
    m_settings->setValue("offsetX", cameraParams.offsetX);
    m_settings->setValue("offsetY", cameraParams.offsetY);

    m_settings->setValue("binningHorizontal", cameraParams.binningHorizontal);
    m_settings->setValue("binningVertical", cameraParams.binningVertical);

    QString binHMode;
    if (cameraParams.binningHorizontalMode == BinningHorizontalMode_Sum)
        binHMode = BaslerConstants::binningModes().at(0);  // "Sum"
    else
        binHMode = BaslerConstants::binningModes().at(1);  // "Average"
    m_settings->setValue("binningHorizontalMode", binHMode);

    QString binVMode;
    if (cameraParams.binningVerticalMode == BinningVerticalMode_Sum)
        binVMode = BaslerConstants::binningModes().at(0);
    else
        binVMode = BaslerConstants::binningModes().at(1);
    m_settings->setValue("binningVerticalMode", binVMode);

    m_settings->endGroup();
    m_settings->sync();
}

const QSettings *BaslerSettings::settings() const { return m_settings; }

QSettings *BaslerSettings::createSettingsObject(QString iniFileName) {
    bool isIniExists = QFile(iniFileName).exists();

    //    if(!isIniExists){
    //        QFile resFile;
    //        QString qrcFileName = ":/4Release/" + projectName + ".ini";
    //        resFile.copy(qrcFileName, currentPath);
    //        QFile fileCopied(currentPath);
    //        fileCopied.setPermissions(QFileDevice::WriteOther);
    //    }
    //    qDebug()<<"ini loading result:"<<isIniExists;

    return new QSettings(iniFileName, QSettings::IniFormat);
}
