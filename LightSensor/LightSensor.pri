# LightSensor.pri
LIGHTSENSOR_PATH = $$PWD
INCLUDEPATH += $$LIGHTSENSOR_PATH

QT += core network

SOURCES += \
    $$LIGHTSENSOR_PATH/LightSensorManager.cpp \
    $$LIGHTSENSOR_PATH/LightSaver.cpp \
    $$LIGHTSENSOR_PATH/LightSettings.cpp \
    $$PWD/UdpLightSensorReader.cpp \
    $$PWD/UdpLightSensorWriter.cpp

HEADERS += \
    $$LIGHTSENSOR_PATH/LightSensorManager.h \
    $$LIGHTSENSOR_PATH/LightSaver.h \
    $$LIGHTSENSOR_PATH/LightSettings.h \
    $$LIGHTSENSOR_PATH/LightTypes.h \
    $$PWD/UdpLightSensorReader.h \
    $$PWD/UdpLightSensorWriter.h

DISTFILES +=

RESOURCES += \
    $$PWD/Light_res.qrc
