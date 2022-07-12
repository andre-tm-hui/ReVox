# Virtual Soundboard and Sampler
A Virtual Soundboard built in C++ for Windows, capable of recording input and system audio, and outputting to a virtual cable (see https://vb-audio.com/Cable/).

It uses a keyboard hook to register key events, and assigns audio events to a set of given keys. Audio events are handled using the portaudio (https://github.com/PortAudio/portaudio/) and libsndfile (https://github.com/libsndfile/libsndfile/) libraries.

The project is made in Qt, using qmake.

Features:
  - Live recording of audio from:
    - An input device
    - System audio (requires WASAPI and Loopback-compatible output device)
  - Input device passthrough to virtual cable
  - Adding/removing/rebinding hotkeys
  - Runs in background
  - Hotkey specific settings:
    - Record only input device
    - Record only loopback device
    - Record both simultaneously
  - Compatible with 1+ channel input devices
    - Samples are recorded as dual-channel audio

Requirements:
  - Tested on Windows 10 only
  - Requires a virtual audio-cable driver (recommended https://vb-audio.com/Cable/)
  
Usage Instructions:
  - Run the application
  - Given that VB-Audio Cable is installed, devices should initialize automatically to system defaults. Change as you see fit.
  - In applications where you want to use the sampler, set the input device to "CABLE Output (VB-Audio Virtual Cable)"
By default, hotkeys for Numpad 0-9 are preset. To change an existing hotkey:
  - Double-click the hotkey text in the app window
  - When it reads "Listening...", tap any key once
  - If the "Listening..." text updates to the key you just pressed, the new hotkey is set
  - If the "Listening..." text does not update after repeated presses of a key, check that the hotkey isn't already set elsewhere
To add a new hotkey, click the "+" button and do as above.
To remove a hotkey, select an existing hotkey by clicking on the hotkey text once, and click the "-" button.
To change a hotkey's settings:
  - Double click the settings cog to the right of the hotkey

One setting that is not as straightforward is the "Pad Loopback" setting. This setting ensures that the loopback recording records even empty sound, i.e. when no system audio is playing. This is useful for synchronizing input and loopback audio recordings.

Build Instructions:
Clone this repo, and add directions to your local portaudio and libsndfile libraries in the .pro file. And then, build.
