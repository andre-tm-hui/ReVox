@echo off
setlocal
rem This script is used to install dependencies for the ReVox project.
rem Pre-requisites: Visual Studio 2022, CMake, Git, curl, tar

rem CHANGE THIS TO YOUR CORRESPONDING VISUAL STUDIO INSTALLATION PATH
set VSVARSALL=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat

if not exist dependencies (
  mkdir dependencies
)
cd dependencies

call "%VSVARSALL%" x64

if not exist jom (
  curl -LO http://download.qt.io/official_releases/jom/jom.zip
  mkdir jom
  tar -xf jom.zip -C ./jom
  del jom.zip
)

set JOM=%cd%\jom\jom.exe

rem Download VB Audio Cable Drivers
if not exist vb-cable (
  curl -LO https://download.vb-audio.com/Download_CABLE/VBCABLE_Driver_Pack43.zip
  tar -xf VBCABLE_Driver_Pack43.zip
  del VBCABLE_Driver_Pack43.zip
  rename VBCABLE_Driver_Pack43 vb-cable
)

rem Clone and build PortAudio from source
if not exist portaudio (
  git clone https://github.com/PortAudio/portaudio.git
)
if not exist portaudio\build (
  cd portaudio
  cmake -G "Visual Studio 17 2022" -A x64 -B build -DPA_BUILD_SHARED_LIBS=1 -DPA_USE_ASIO=0 -DPA_USE_DS=0 -DPA_USE_WASAPI=1 -DPA_USE_WDMKS=0 -DPA_USE_WDMKS_DEVICE_INFO=0 -DPA_USE_WMME=0
  cd build
  cmake --build . --config Release
  cmake --build . --config Debug
  cd ../..
)

rem Clone and build spdlog from source
if not exist spdlog (
  git clone https://github.com/gabime/spdlog.git
)
if not exist spdlog\build (
  cd spdlog
  cmake -G "Visual Studio 17 2022" -A x64 -B build -DSPDLOG_BUILD_SHARED=1
  cd build
  cmake --build . --config Release
  cmake --build . --config Debug
  cd ../..
)

rem Download pre-built fftw libraries
if not exist fftw (
  curl -LO https://www.fftw.org/fftw-3.3.10.tar.gz
  tar -xf fftw-3.3.10.tar.gz
  del fftw-3.3.10.tar.gz
  rename fftw-3.3.10 fftw
)
if not exist fftw\build (
  cd fftw
  cmake -G "Visual Studio 17 2022" -A x64 -B build -DBUILD_SHARED_LIBS=1 -DENABLE_FLOAT=1
  cd build
  cmake --build . --config Release
  cmake --build . --config Debug
  cd ../..
)

rem Clone and build github-releases-autoupdater from source
if not exist cpp-template-utils (
  git clone https://github.com/VioletGiraffe/cpp-template-utils.git
)
if not exist github-releases-autoupdater (
  git clone https://github.com/andre-tm-hui/github-releases-autoupdater.git
)
cd github-releases-autoupdater
qmake github-releases-autoupdater.pro
"%JOM%" 
qmake CONFIG+=debug github-releases-autoupdater.pro
"%JOM%"
cd ..

if not exist SingleApplication (
  git clone https://github.com/itay-grudev/SingleApplication.git
)

rem Download pre-built libsndfile libraries
if not exist libsndfile (
  curl -LO https://github.com/libsndfile/libsndfile/releases/download/1.2.2/libsndfile-1.2.2-win64.zip 
  tar -xf libsndfile-1.2.2-win64.zip
  del libsndfile-1.2.2-win64.zip
  rename libsndfile-1.2.2-win64 libsndfile
)

rem Download pre-built libsamplerate libraries
if not exist libsamplerate (
  curl -LO https://github.com/libsndfile/libsamplerate/releases/download/0.2.2/libsamplerate-0.2.2-win64.zip 
  tar -xf libsamplerate-0.2.2-win64.zip
  del libsamplerate-0.2.2-win64.zip
  rename libsamplerate-0.2.2-win64 libsamplerate
)

rem Download nlohmann/json header-only library
if not exist nlohmann (
  curl -LO https://github.com/nlohmann/json/releases/download/v3.11.3/include.zip 
  tar -xf include.zip --strip-components=1 single_include/nlohmann
  del include.zip
)

endlocal