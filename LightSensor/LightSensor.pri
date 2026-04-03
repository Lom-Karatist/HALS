# LightSensor.pri
LIGHTSENSOR_PATH = $$PWD
INCLUDEPATH += $$LIGHTSENSOR_PATH

QT += core

SOURCES += \
    $$LIGHTSENSOR_PATH/LightSensorApi.cpp \
    $$LIGHTSENSOR_PATH/LightSensorManager.cpp \
    $$LIGHTSENSOR_PATH/LightSaver.cpp \
    $$LIGHTSENSOR_PATH/LightSettings.cpp

HEADERS += \
    $$LIGHTSENSOR_PATH/LightSensorApi.h \
    $$LIGHTSENSOR_PATH/LightSensorManager.h \
    $$LIGHTSENSOR_PATH/LightSaver.h \
    $$LIGHTSENSOR_PATH/LightSettings.h \
    $$LIGHTSENSOR_PATH/LightTypes.h

DISTFILES +=

RESOURCES += \
    $$PWD/Light_res.qrc
