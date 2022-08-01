# Virtual Soundboard and Sampler
A Virtual Soundboard built in C++ for Windows, capable of recording input and system audio, and outputting to a virtual cable (see https://vb-audio.com/Cable/).

It uses a keyboard hook to register key events, and assigns audio events to a set of given keys. Audio events are handled using the portaudio (https://github.com/PortAudio/portaudio/) and libsndfile (https://github.com/libsndfile/libsndfile/) libraries. Some DSP, specifically Fast Fourier Transforms, are handled by the FFTW library (https://www.fftw.org/). Settings I/O is handled using nlohmann's json library (https://github.com/nlohmann/json).

The reverb algorithm is Freeverb, written by Jezar, with minor fixes (NaN correction when floats become too small).

The project is made in Qt, using qmake.

# Features
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
    - Use hotkeys to toggle effects on the input device:
      - Reverb
	  - Autotune/Hardtune
	  - More to come
	- Each effect toggle saves a configuration of all available effects and applies them on keypress, so you can quickly change to complex configurations of effects
  - Compatible with 1+ channel input devices
    - Samples are recorded as dual-channel audio

# Requirements
  - Tested on Windows 10 only
  - Requires a virtual audio-cable driver (recommended https://vb-audio.com/Cable/)
  
# Usage Instructions
  - Run the application
  - Given that VB-Audio Cable is installed, devices should initialize automatically to system defaults. Change as you see fit.
  - In applications where you want to use the sampler, set the input device to "CABLE Output (VB-Audio Virtual Cable)"
To add a new hotkey, click the "+" button under the Soundboard/VoiceFX list and do the following
  - When a new entry is added and reading "Listening..." in the 2nd column, tap any key once
  - If the "Listening..." text updates to the key you just pressed, the new hotkey is set
  - If the "Listening..." text does not update after repeated presses of a key, that hotkey may not be valid*
To change a hotkey, click on the hotkey (2nd column), and repeat as above.
To remove a hotkey, select an existing hotkey by clicking on the hotkey text once, and click the "-" button.
To change a hotkey's settings:
  - Double click the settings cog to the right of the hotkey (in the 3rd column)
Each hotkey comes with a customizable label in the leftmost column.
  
To add pre-recorded audio clips (.WAV, dual-channel only):
  - Navigate to %appdata%/Virtual Soundboard and Sampler/samples
  - Add your file to the folder
  - Rename it to match a hotkey, e.g. "96.wav"
    - Refer to https://docs.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
    - Keycodes given above are in hexadecimal, so convert them to decimal (https://www.rapidtables.com/convert/number/hex-to-decimal.html)

One setting that is not as straightforward is the "Pad Loopback" setting. This setting ensures that the loopback recording records even empty sound, i.e. when no system audio is playing. This is useful for synchronizing input and loopback audio recordings.

*Hotkeys are invalid if they are already in use

# Build Instructions
Clone this repo, and add directions to your local portaudio, libsndfile and fftw libraries in the .pro file. And then, build.
