QT       += core gui concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
RC_FILE = recource.rc

include(Basler/Basler.pri)
include(GPS/solutions/gps_device/gps_device.pri)

win32 {
    LIBS += -lwbemuuid
}

SOURCES += \
    BaseTools/IniFileLoader.cpp \
    BaseTools/QrcFilesRestorer.cpp \
    Components/CpuTemperatureController.cpp \
    Components/FlightTaskModule.cpp \
    Components/HalsFacade.cpp \
    Components/Logger.cpp \
    Components/UsbChecker.cpp \
    GUI/OverlayLabel.cpp \
    GUI/SensorCharacteristicsForm.cpp \
    GUI/StatusIndicator.cpp \
    main.cpp \
    GUI/HalsWindow.cpp

HEADERS += \
    BaseTools/IniFileLoader.h \
    BaseTools/QrcFilesRestorer.h \
    Components/CpuTemperatureController.h \
    Components/FlightTaskModule.h \
    Components/HalsFacade.h \
    Components/Logger.h \
    Components/UsbChecker.h \
    GUI/HalsWindow.h \
    GUI/OverlayLabel.h \
    GUI/SensorCharacteristicsForm.h \
    GUI/StatusIndicator.h \
    version.h

FORMS += \
    GUI/HalsWindow.ui \
    GUI/SensorCharacteristicsForm.ui \
    GUI/StatusIndicator.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    GUI/res.qrc
