QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
RC_FILE = recource.rc

include(Basler/Basler.pri)

SOURCES += \
    main.cpp \
    GUI/HalsWindow.cpp

HEADERS += \
    GUI/HalsWindow.h \
    version.h

FORMS += \
    GUI/HalsWindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
