# ReVox
## [Download Here!](https://github.com/onglez/ReVox/releases)
<p align="center"><image src="https://github.com/andre-tm-hui/ReVox/blob/master/images/intro.gif" width="60%"></image></p>

A Virtual Soundboard, Sampler and Voice Changer packaged into a single program. Get creative in your voice chats/streams.

### [Get Started](https://github.com/onglez/ReVox/wiki/Installation)
### [Join the Discord](https://discord.gg/ugv5aMSm63)
### [Follow the Development](https://trello.com/b/cNNxJTYr/revox)

#### Note:
Some antivirus software may flag ReVox as a virus. I can guarantee that any version of ReVox downloaded from official sources (GitHub, Discord Releases channel) is completely safe. ReVox does not send any information anywhere, and only accesses the internet when checking for updates.

This can possibly be fixed with money, but this is a non-profit passion project with no funding.

# Features
  - Realtime recording of audio from:
    - An input device (microphone, audio interface, headset etc.)
    - System audio (web browser, Discord etc.)
    - Pre-recording buffer - record up to 5 extra seconds before pressing the record button
  - Output to voice chat and recording apps
  - Adding/removing/rebinding hotkeys
  - Runs in background
  - Highly customizable:
    - Soundboard:
      - Crop clips
      - Change clip volume
      - Record input device and system audio separately/together
      - Make the clip spammable
    - Voice Changer:
      - FX specific settings
      - Mix and match FX - toggle multiple FXs through a single hotkey
  - Monitor your own audio
  - [Input Blocking](https://github.com/onglez/ReVox/wiki/Usage#input-blocking) - press a hotkey and only have the sound play/FX activate. Bind hotkeys freely without worrying about what the original key does.
    - *Requires separate add-on
    - **May not work with every game

## Requirements
  - Windows 10/11
  - [VB-Audio Virtual Cable](https://vb-audio.com/Cable/)
  - [Microsoft Visual C++ Redistributable](https://aka.ms/vs/17/release/vc_redist.x64.exe)

## Build Instructions
### Prerequisites
  - [Qt Creator](https://qt.io)
  - [PortAudio](https://github.com/PortAudio/portaudio/)
  - [Libsndfile](https://github.com/libsndfile/libsndfile/)
  - [Libsamplerate](https://github.com/libsndfile/libsamplerate)
  - [FFTW](https://www.fftw.org/)
  - [nlohmann's json library](https://github.com/nlohmann/json)
  - [GitHub Releases Autoupdater](https://github.com/VioletGiraffe/github-releases-autoupdater)
### Instructions:
1. Clone the repo
2. In ReVox.pro, relink the above libraries to your local libraries/binaries
  a. Copy the nlohmann headers folder into the root of the project folder, or relink them in the ReVox.pro file
3. Build using MinGW

## Credits
Uses the following:
  - Qt Framework (Open Source) for UI
  - PortAudio to deal with audio streams
  - Libsndfile for audio I/O
  - Libsamplerate for resampling of audio
  - FFTW for efficiently calculating fast fourier transforms
  - nlohmann's json library for saving user configs in human-readable formats
  - Freeverb, written by Jezar, for a solid open-source reverb algorithm
  - GitHub Releases Autoupdater for scraping new updates from GitHub
  - [SingleApplication](https://github.com/itay-grudev/SingleApplication) to prevent users from having more than one instance of ReVox running simultaneously
  
### Licensing
Licensed under GPLv3 Open-Source License
