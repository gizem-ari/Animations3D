QT       += core gui opengl widgets openglwidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += SettingsWindow/HyperCubeSettingsWindow
INCLUDEPATH += SettingsWindow/StaticCubeSettingsWindow
INCLUDEPATH += SettingsWindow/Text3DSettingsWindow
INCLUDEPATH += SettingsWindow/SolidText3DSettingsWindow
INCLUDEPATH += SettingsWindow/LinearWaveSettingsWindow
INCLUDEPATH += SettingsWindow/WaveFountain3DSettingsWindow
INCLUDEPATH += SettingsWindow/PendulumWaveSettingsWindow
INCLUDEPATH += SettingsWindow/KineticRainSettingsWindow

SOURCES += \
    SettingsWindow/KineticRainSettingsWindow/kineticrainsettingswindow.cpp \
    SettingsWindow/LinearWaveSettingsWindow/linearwavesettingswindow.cpp \
    SettingsWindow/PendulumWaveSettingsWindow/pendulumwavesettingswindow.cpp \
    SettingsWindow/SolidText3DSettingsWindow/solidtext3dsettingswindow.cpp \
    SettingsWindow/Text3DSettingsWindow/text3dsettingswindow.cpp \
    SettingsWindow/WaveFountain3DSettingsWindow/wavefountain3dsettingswindow.cpp \
    animationalgorithms.cpp \
    core.cpp \
    framerendererthread.cpp \
    main.cpp \
    mainwindow.cpp\
    SettingsWindow/HyperCubeSettingsWindow/hypercubesettingswindow.cpp \
    SettingsWindow/StaticCubeSettingsWindow/staticcubesettingswindow.cpp \
    simulationwidget.cpp

HEADERS += \
    SettingsWindow/HyperCubeSettingsWindow/hypercubesettingswindow.h \
    SettingsWindow/KineticRainSettingsWindow/kineticrainsettingswindow.h \
    SettingsWindow/LinearWaveSettingsWindow/linearwavesettingswindow.h \
    SettingsWindow/PendulumWaveSettingsWindow/pendulumwavesettingswindow.h \
    SettingsWindow/SolidText3DSettingsWindow/solidtext3dsettingswindow.h \
    SettingsWindow/StaticCubeSettingsWindow/staticcubesettingswindow.h \
    SettingsWindow/Text3DSettingsWindow/text3dsettingswindow.h \
    SettingsWindow/WaveFountain3DSettingsWindow/wavefountain3dsettingswindow.h \
    animationalgorithms.h \
    core.h \
    framerendererthread.h \
    mainwindow.h \
    simulationwidget.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
