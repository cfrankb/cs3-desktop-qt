QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

INCLUDEPATH += shared/ headers/
CONFIG += c++17
DEFINES += USE_QFILE=1

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    actor.cpp \
    animator.cpp \
    game.cpp \
    gamemixin.cpp \
    gamewidget.cpp \
    level.cpp \
    main.cpp \
    mainwindow.cpp \
    map.cpp \
    maparch.cpp \
    #runtime.cpp \
    shared/DotArray.cpp \
    shared/FileWrap.cpp \
    shared/Frame.cpp \
    shared/FrameSet.cpp \
    shared/PngMagic.cpp \
    shared/helper.cpp \
    shared/implementers/mu_sdl.cpp \
    shared/implementers/sn_sdl.cpp \
    shared/qtgui/qfilewrap.cpp \
    shared/qtgui/qthelper.cpp \
    tilesdata.cpp

HEADERS += \
    actor.h \
    animator.h \
    animzdata.h \
    anniedata.h \
    game.h \
    gamemixin.h \
    gamewidget.h \
    level.h \
    mainwindow.h \
    map.h \
    maparch.h \
    #runtime.h \
    shared/CRC.h \
    shared/DotArray.h \
    shared/FileWrap.h \
    shared/Frame.h \
    shared/FrameSet.h \
    shared/IFile.h \
    shared/ISerial.h \
    shared/PngMagic.h \
    shared/glhelper.h \
    shared/helper.h \
    shared/implementers/mu_sdl.h \
    shared/implementers/sn_sdl.h \
    shared/interfaces/IMusic.h \
    shared/interfaces/ISound.h \
    shared/qtgui/cheat.h \
    shared/qtgui/qfilewrap.h \
    shared/qtgui/qthelper.h \
    sounds.h \
    sprtypes.h \
    tilesdata.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    cs3_desktop_qt.qrc

unix:LIBS += -lz
win32:LIBS += -L"libs" -lzlib
unix:LIBS += -lSDL2_mixer -lSDL2

QMAKE_CXXFLAGS_RELEASE += -std=c++17 -O3
QMAKE_CXXFLAGS_DEBUG += -std=c++17 -g3
