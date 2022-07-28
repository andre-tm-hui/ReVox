QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++20

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    source/audio/dsp/comb.cpp \
    source/audio/audiomanager.cpp \
    source/audio/dsp/allpass.cpp \
    source/audio/dsp/frequency.cpp \
    source/audio/dsp/pitchshift.cpp \
    source/audio/dsp/revmodel.cpp \
    source/audio/dsp/stft.cpp \
    source/audio/streams/audiostream.cpp \
    source/audio/callbacks.cpp \
    source/qt/keybindsettings.cpp \
    source/hook/keyboardlistener.cpp \
    source/main.cpp \
    source/qt/mainwindow.cpp \
    source/audio/streams/passthrough.cpp \
    source/audio/streams/player.cpp \
    source/audio/streams/recorder.cpp

HEADERS += \
    headers/allpass.h \
    headers/audiomanager.h \
    headers/audiostream.h \
    headers/callbacks.h \
    headers/comb.h \
    headers/denormals.h \
    headers/frequency.h \
    headers/keybindsettings.h \
    headers/keyboardlistener.h \
    headers/mainwindow.h \
    headers/passthrough.h \
    headers/pitchshift.h \
    headers/player.h \
    headers/pvdat.h \
    headers/recorder.h \
    headers/revmodel.h \
    headers/revtuning.h \
    headers/stft.h \
    headers/vkcodenames.h \
    headers/windowfunctions.h

INCLUDEPATH += headers

FORMS += \
    qtui/keybindsettings.ui \
    qtui/mainwindow.ui

TRANSLATIONS += \
    VirtualSnS_en_001.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../portaudio/msvc/x64/release/ -lportaudio_x64
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../portaudio/msvc/x64/release/ -lportaudio_x64

INCLUDEPATH += $$PWD/../portaudio/include
DEPENDPATH += $$PWD/../portaudio/msvc/x64/Release

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/'../../C++ Libraries/libsndfile-1.1.0-win64/lib/' -lsndfile
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/'../../C++ Libraries/libsndfile-1.1.0-win64/lib/' -lsndfile

INCLUDEPATH += $$PWD/'../../C++ Libraries/libsndfile-1.1.0-win64/include'
DEPENDPATH += $$PWD/'../../C++ Libraries/libsndfile-1.1.0-win64/bin'

RESOURCES += \
    appicon.qrc \
    settingsicon.qrc

include(singleapplication/singleapplication.pri)
DEFINES += QAPPLICATION_CLASS=QApplication

win32: LIBS += -L$$PWD/'../../C++ Libraries/fftw/' -llibfftw3f-3

INCLUDEPATH += $$PWD/'../../C++ Libraries/fftw'
DEPENDPATH += $$PWD/'../../C++ Libraries/fftw'
