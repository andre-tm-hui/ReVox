QT       += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++20 \
    qml_debug

RC_ICONS = icon.ico


# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    audiofx/lib/pitchshift/pitchshifter.cpp \
    main.cpp \
    audiofx/autotuner.cpp \
    audiofx/repitcher.cpp \
    audiofx/reverberator.cpp \
    audiofx/lib/pitchshift/frequency.cpp \
    audiofx/lib/pitchshift/pitchshift.cpp \
    audiofx/lib/pitchshift/stft.cpp \
    audiofx/lib/reverb/allpass.cpp \
    audiofx/lib/reverb/comb.cpp \
    audiofx/lib/reverb/revmodel.cpp \
    backend/keyboardlistener.cpp \
    backend/streams/audiostream.cpp \
    backend/streams/callbacks.cpp \
    backend/streams/cleanoutput.cpp \
    backend/streams/monitor.cpp \
    backend/streams/noisegenerator.cpp \
    backend/streams/passthrough.cpp \
    backend/streams/player.cpp \
    interface/baseinterface.cpp \
    interface/basemanager.cpp \
    interface/fxmanager.cpp \
    interface/maininterface.cpp \
    interface/soundboardmanager.cpp \
    ui/hud.cpp \
    ui/mainwindow.cpp \
    ui/onboarding.cpp \
    ui/menus/fxmenu.cpp \
    ui/menus/soundboardmenu.cpp \
    ui/titlebar.cpp \
    ui/components/cropper.cpp \
    ui/components/fxtab.cpp \
    ui/menus/settingsmenu.cpp \
    ui/util/keypresseater.cpp \
    ui/util/qttransitions.cpp \
    ui/widgets/bindablebutton.cpp \
    ui/widgets/customdial.cpp \
    ui/widgets/hotkeylistitemwidget.cpp \
    ui/widgets/hotkeylistwidget.cpp \
    ui/widgets/monitorslider.cpp \
    ui/widgets/switch.cpp \
    ui/widgets/fx/fxdial.cpp \
    ui/widgets/fx/fxkeypicker.cpp \
    ui/widgets/fx/fxparam.cpp \
    ui/widgets/soundboard/clickthroughslider.cpp \
    ui/widgets/soundboard/waveformviewer.cpp

HEADERS += \
    audiofx/iaudiofx.h \
    audiofx/autotuner.h \
    audiofx/lib/pitchshift/pitchshifter.h \
    audiofx/repitcher.h \
    audiofx/reverberator.h \
    audiofx/lib/pitchshift/frequency.h \
    audiofx/lib/pitchshift/pitchshift.h \
    audiofx/lib/pitchshift/stft.h \
    audiofx/lib/pitchshift/pvdat.h \
    audiofx/lib/pitchshift/windowfunctions.h \
    audiofx/lib/reverb/allpass.h \
    audiofx/lib/reverb/comb.h \
    audiofx/lib/reverb/revmodel.h \
    audiofx/lib/reverb/denormals.h \
    audiofx/lib/reverb/revtuning.h \
    backend/keyboardlistener.h \
    backend/vkcodenames.h \
    backend/streams/audiostream.h \
    backend/streams/callbacks.h \
    backend/streams/cleanoutput.h \
    backend/streams/monitor.h \
    backend/streams/noisegenerator.h \
    backend/streams/passthrough.h \
    backend/streams/player.h \
    interface/baseinterface.h \
    interface/basemanager.h \
    interface/fxmanager.h \
    interface/maininterface.h \
    interface/soundboardmanager.h \
    ui/hud.h \
    ui/mainwindow.h \
    ui/onboarding.h \
    ui/menus/fxmenu.h \
    ui/menus/soundboardmenu.h \
    ui/titlebar.h \
    ui/components/cropper.h \
    ui/components/fxtab.h \
    ui/menus/settingsmenu.h \
    ui/util/keypresseater.h \
    ui/util/qttransitions.h \
    ui/widgets/bindablebutton.h \
    ui/widgets/customdial.h \
    ui/widgets/hotkeylistitemwidget.h \
    ui/widgets/hotkeylistwidget.h \
    ui/widgets/monitorslider.h \
    ui/widgets/switch.h \
    ui/widgets/style.h \
    ui/widgets/fx/fxdial.h \
    ui/widgets/fx/fxkeypicker.h \
    ui/widgets/fx/fxparam.h \
    ui/widgets/soundboard/clickthroughslider.h \
    ui/widgets/soundboard/waveformviewer.h \
    nlohmann/json.hpp \
    util/ringbuffer.h

INCLUDEPATH += headers

FORMS += \
    ui/hud.ui \
    ui/mainwindow.ui \
    ui/menus/fxmenu.ui \
    ui/menus/soundboardmenu.ui \
    ui/onboarding.ui \
    ui/titlebar.ui \
    ui/components/fxtab.ui \
    ui/menus/settingsmenu.ui \
    ui/widgets/monitorslider.ui \
    ui/widgets/fx/fxdial.ui \
    ui/widgets/fx/fxkeypicker.ui

CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    rc.qrc

include(singleapplication/singleapplication.pri)
DEFINES += QAPPLICATION_CLASS=QApplication

win32: LIBS += -L$$PWD/'../../C++ Libraries/fftw/' -llibfftw3f-3

INCLUDEPATH += $$PWD/'../../C++ Libraries/fftw'
DEPENDPATH += $$PWD/'../../C++ Libraries/fftw'

win32: LIBS += -L$$PWD/'../../C++ Libraries/libsamplerate-0.2.2-win64/lib/' -lsamplerate

INCLUDEPATH += $$PWD/'../../C++ Libraries/libsamplerate-0.2.2-win64/include'
DEPENDPATH += $$PWD/'../../C++ Libraries/libsamplerate-0.2.2-win64/bin'


LIBS += -lole32 \
        -luser32 \
        -lcfgmgr32 \

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../bin/release/x64/ -lautoupdater
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../bin/debug/x64/ -lautoupdater

INCLUDEPATH += $$PWD/../github-releases-autoupdater/src \
               $$PWD/../github-releases-autoupdater/src/updaterUI
DEPENDPATH += $$PWD/../build/release/x64/autoupdater


win32: LIBS += -lhid



win32: LIBS += -L$$PWD/'../../C++ Libraries/libsndfile-1.1.0-win64/bin/' -lsndfile

INCLUDEPATH += $$PWD/'../../C++ Libraries/libsndfile-1.1.0-win64/include'
DEPENDPATH += $$PWD/'../../C++ Libraries/libsndfile-1.1.0-win64/include'

win32: LIBS += -L$$PWD/../portaudio/msvc/x64/Release/ -lportaudio_x64

INCLUDEPATH += $$PWD/../portaudio/include
DEPENDPATH += $$PWD/../portaudio/include
