QT       += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++20 \
    qml_debug

RC_ICONS = icon.ico

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    onboarding.cpp \
    source/qt/widgets/errdialog.cpp \
    source/qt/menus/voicefx/autotunetab.cpp \
    source/qt/menus/fxmenu.cpp \
    source/qt/hud.cpp \
    source/qt/menus/voicefx/pitchtab.cpp \
    source/qt/menus/voicefx/reverbtab.cpp \
    source/qt/menus/settingsmenu.cpp \
    source/qt/menus/soundboardmenu.cpp \
    source/qt/rc/qttransitions.cpp \
    source/audio/streams/cleanoutput.cpp \
    source/audio/streams/monitor.cpp \
    source/audio/streams/noisegenerator.cpp \
    source/audio/dsp/comb.cpp \
    source/audio/audiomanager.cpp \
    source/audio/dsp/allpass.cpp \
    source/audio/dsp/frequency.cpp \
    source/audio/dsp/pitchshift.cpp \
    source/audio/dsp/revmodel.cpp \
    source/audio/dsp/stft.cpp \
    source/audio/streams/audiostream.cpp \
    source/audio/callbacks.cpp \
    source/hook/keyboardlistener.cpp \
    source/main.cpp \
    source/qt/mainwindow.cpp \
    source/audio/streams/passthrough.cpp \
    source/audio/streams/player.cpp \
    source/qt/rc/hotkeyitem.cpp \
    source/qt/rc/keypresseater.cpp \
    source/qt/widgets/customdial.cpp \
    source/qt/widgets/switch.cpp \
    source/qt/widgets/waveformviewer.cpp \
    source/qt/titlebar.cpp

HEADERS += \
    headers/errdialog.h \
    headers/autotunetab.h \
    headers/fxmenu.h \
    headers/cleanoutput.h \
    headers/customdial.h \
    headers/allpass.h \
    headers/audiomanager.h \
    headers/audiostream.h \
    headers/callbacks.h \
    headers/comb.h \
    headers/denormals.h \
    headers/frequency.h \
    headers/hotkeyitem.h \
    headers/keyboardlistener.h \
    headers/keypresseater.h \
    headers/mainwindow.h \
    headers/monitor.h \
    headers/noisegenerator.h \
    headers/passthrough.h \
    headers/pitchshift.h \
    headers/player.h \
    headers/pvdat.h \
    headers/qttransitions.h \
    headers/revmodel.h \
    headers/revtuning.h \
    headers/stft.h \
    headers/style.h \
    headers/switch.h \
    headers/vkcodenames.h \
    headers/waveformviewer.h \
    headers/windowfunctions.h \
    headers/hud.h \
    headers/pitchtab.h \
    headers/reverbtab.h \
    headers/settingsmenu.h \
    headers/soundboardmenu.h \
    headers/titlebar.h \
    nlohmann/json.hpp \
    onboarding.h

INCLUDEPATH += headers

FORMS += \
    onboarding.ui \
    qtui/errdialog.ui \
    qtui/autotunetab.ui \
    qtui/fxmenu.ui \
    qtui/hud.ui \
    qtui/pitchtab.ui \
    qtui/mainwindow.ui \
    qtui/reverbtab.ui \
    qtui/settingsmenu.ui \
    qtui/soundboardmenu.ui \
    qtui/titlebar.ui

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
    appicon.qrc

include(singleapplication/singleapplication.pri)
DEFINES += QAPPLICATION_CLASS=QApplication

win32: LIBS += -L$$PWD/'../../C++ Libraries/fftw/' -llibfftw3f-3

INCLUDEPATH += $$PWD/'../../C++ Libraries/fftw'
DEPENDPATH += $$PWD/'../../C++ Libraries/fftw'

win32: LIBS += -L$$PWD/'../../C++ Libraries/libsamplerate-0.2.2-win64/lib/' -lsamplerate

INCLUDEPATH += $$PWD/'../../C++ Libraries/libsamplerate-0.2.2-win64/include'
DEPENDPATH += $$PWD/'../../C++ Libraries/libsamplerate-0.2.2-win64/bin'


LIBS += -lole32

win32: LIBS += -L$$PWD/../bin/release/x64/ -lautoupdater

INCLUDEPATH += $$PWD/../github-releases-autoupdater/src \
               $$PWD/../github-releases-autoupdater/src/updaterUI
DEPENDPATH += $$PWD/../build/release/x64/autoupdater
