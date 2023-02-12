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
    ui/widgets/fx/fxswitch.cpp \
    ui/widgets/hotkeylistitemwidget.cpp \
    ui/widgets/hotkeylistwidget.cpp \
    ui/widgets/monitorslider.cpp \
    ui/widgets/switch.cpp \
    ui/widgets/fx/fxdial.cpp \
    ui/widgets/fx/fxkeypicker.cpp \
    ui/widgets/fx/fxparam.cpp \
    ui/widgets/soundboard/clickthroughslider.cpp \
    ui/widgets/soundboard/waveformviewer.cpp \
    util/loggableobject.cpp

HEADERS += \
    audiofx/iaudiofx.h \
    audiofx/autotuner.h \
    audiofx/lib/pitchshift/pitchshifter.h \
    audiofx/repitcher.h \
    audiofx/reverberator.h \
    audiofx/lib/pitchshift/frequency.h \
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
    spdlog/async.h \
    spdlog/async_logger-inl.h \
    spdlog/async_logger.h \
    spdlog/cfg/argv.h \
    spdlog/cfg/env.h \
    spdlog/cfg/helpers-inl.h \
    spdlog/cfg/helpers.h \
    spdlog/common-inl.h \
    spdlog/common.h \
    spdlog/details/backtracer-inl.h \
    spdlog/details/backtracer.h \
    spdlog/details/circular_q.h \
    spdlog/details/console_globals.h \
    spdlog/details/file_helper-inl.h \
    spdlog/details/file_helper.h \
    spdlog/details/fmt_helper.h \
    spdlog/details/log_msg-inl.h \
    spdlog/details/log_msg.h \
    spdlog/details/log_msg_buffer-inl.h \
    spdlog/details/log_msg_buffer.h \
    spdlog/details/mpmc_blocking_q.h \
    spdlog/details/null_mutex.h \
    spdlog/details/os-inl.h \
    spdlog/details/os.h \
    spdlog/details/periodic_worker-inl.h \
    spdlog/details/periodic_worker.h \
    spdlog/details/registry-inl.h \
    spdlog/details/registry.h \
    spdlog/details/synchronous_factory.h \
    spdlog/details/tcp_client-windows.h \
    spdlog/details/tcp_client.h \
    spdlog/details/thread_pool-inl.h \
    spdlog/details/thread_pool.h \
    spdlog/details/udp_client-windows.h \
    spdlog/details/udp_client.h \
    spdlog/details/windows_include.h \
    spdlog/fmt/bin_to_hex.h \
    spdlog/fmt/bundled/args.h \
    spdlog/fmt/bundled/chrono.h \
    spdlog/fmt/bundled/color.h \
    spdlog/fmt/bundled/compile.h \
    spdlog/fmt/bundled/core.h \
    spdlog/fmt/bundled/format-inl.h \
    spdlog/fmt/bundled/format.h \
    spdlog/fmt/bundled/locale.h \
    spdlog/fmt/bundled/os.h \
    spdlog/fmt/bundled/ostream.h \
    spdlog/fmt/bundled/printf.h \
    spdlog/fmt/bundled/ranges.h \
    spdlog/fmt/bundled/xchar.h \
    spdlog/fmt/chrono.h \
    spdlog/fmt/compile.h \
    spdlog/fmt/fmt.h \
    spdlog/fmt/ostr.h \
    spdlog/fmt/ranges.h \
    spdlog/fmt/xchar.h \
    spdlog/formatter.h \
    spdlog/fwd.h \
    spdlog/logger-inl.h \
    spdlog/logger.h \
    spdlog/pattern_formatter-inl.h \
    spdlog/pattern_formatter.h \
    spdlog/sinks/android_sink.h \
    spdlog/sinks/ansicolor_sink-inl.h \
    spdlog/sinks/ansicolor_sink.h \
    spdlog/sinks/base_sink-inl.h \
    spdlog/sinks/base_sink.h \
    spdlog/sinks/basic_file_sink-inl.h \
    spdlog/sinks/basic_file_sink.h \
    spdlog/sinks/daily_file_sink.h \
    spdlog/sinks/dist_sink.h \
    spdlog/sinks/dup_filter_sink.h \
    spdlog/sinks/hourly_file_sink.h \
    spdlog/sinks/mongo_sink.h \
    spdlog/sinks/msvc_sink.h \
    spdlog/sinks/null_sink.h \
    spdlog/sinks/ostream_sink.h \
    spdlog/sinks/qt_sinks.h \
    spdlog/sinks/ringbuffer_sink.h \
    spdlog/sinks/rotating_file_sink-inl.h \
    spdlog/sinks/rotating_file_sink.h \
    spdlog/sinks/sink-inl.h \
    spdlog/sinks/sink.h \
    spdlog/sinks/stdout_color_sinks-inl.h \
    spdlog/sinks/stdout_color_sinks.h \
    spdlog/sinks/stdout_sinks-inl.h \
    spdlog/sinks/stdout_sinks.h \
    spdlog/sinks/syslog_sink.h \
    spdlog/sinks/systemd_sink.h \
    spdlog/sinks/tcp_sink.h \
    spdlog/sinks/udp_sink.h \
    spdlog/sinks/win_eventlog_sink.h \
    spdlog/sinks/wincolor_sink-inl.h \
    spdlog/sinks/wincolor_sink.h \
    spdlog/spdlog-inl.h \
    spdlog/spdlog.h \
    spdlog/stopwatch.h \
    spdlog/tweakme.h \
    spdlog/version.h \
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
    ui/widgets/fx/fxswitch.h \
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
    util/loggableobject.h \
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
    ui/widgets/fx/fxswitch.ui \
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

DISTFILES += \
  README.md \
  spdlog/fmt/bundled/fmt.license.rst
