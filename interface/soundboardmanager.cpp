#include "soundboardmanager.h"

SoundboardManager::SoundboardManager(std::string rootDir, int sampleRate)
    : BaseManager(rootDir, "FXManager"), sampleRate(sampleRate) {
  dataPath = "soundboardSettings.json";
  settings = R"({
        "monitor": 100,
        "recordOver": {
            "keycode": -1,
            "deviceName": "" },
        "hotkeys": {},
        "padding": 0
    })"_json;
  defaultObj = R"({
        "keycode": -1,
        "deviceName": "",
        "label": "",
        "recordInput": true,
        "recordLoopback": true,
        "startAt": 0,
        "endAt": -1,
        "maxCopies": -1,
        "volume": 1.0,
        "editable": true
    })"_json;
  LoadSettings();

  log(INFO, "SoundboardManager initiated");
}

void SoundboardManager::StartEvent(std::string path, int event) {
  if (path.find("hotkeys") != path.npos) {
    std::string idx = path.substr(path.find("/") + 1);
    json bindSettings = settings["hotkeys"][idx];
    if (bindSettings.empty()) return;
    if (event == 0) {
        holding++;
        Play(idx, event);
    }
    else {
        holding = 0;
        if (recording) {
            StopRecording();
        }
        else if (modifier) {
            Stop(idx);
        }
    }
  } else {
    if (path == "recordOver") {
        if (event == 0) {
            modifier = true;
        }
        else {
            modifier = false;
        }
    }
  }
}

void SoundboardManager::Record(std::string idx) {
  if (recording || stopping) return;
  json bindSettings = settings["hotkeys"][idx];

  if (bindSettings["recordInput"] && passthrough != nullptr) {
    log(INFO, "Sample " + idx + " - Recording input");
    passthrough->Record(std::stoi(idx));
  }
  if (bindSettings["recordLoopback"] && player != nullptr) {
    log(INFO, "Sample " + idx + " - Recording system");
    player->Record(std::stoi(idx));
  }

  currHotkey = idx;
  recording = true;
}

void SoundboardManager::StopRecording() {
  if (recording && !stopping) {
    stopping = true;
    std::thread t(&SoundboardManager::StopRecordingAfter, this);
    t.detach();
  }
}

void SoundboardManager::StopRecordingAfter() {
  Sleep(settings["padding"].get<int>());
  log(INFO, "Recording stopped");

  if (passthrough != nullptr) passthrough->Stop();
  if (monitor != nullptr) player->Stop();

  stopping = false;
  recording = false;
  if (wv != nullptr)
    wv->refresh(rootDir + "samples/" + currHotkey + ".mp3", wv);
}

void SoundboardManager::Play(std::string idx, int ev) {
  if (player != nullptr && !recording) {
    bool playable = player->CanPlay(std::stoi(idx));
    if (modifier && holding > 1 || !playable)
      Record(idx);
    else if (!modifier && playable) {
      log(INFO, "Sample " + idx + " - Playing");
      player->Play(std::stoi(idx), settings["hotkeys"][idx]);
    }
  }
}

void SoundboardManager::StopAll() {
  log(INFO, "Stopping all playback");
  player->StopAll();
}

void SoundboardManager::Stop(std::string idx) {
    log(INFO, "Stopping sample " + idx);
    player->StopPlaying(std::stoi(idx));
}

void SoundboardManager::OverrideSound(std::string fname, int idx) {
  log(INFO, "Key " + std::to_string(idx) + " - Overriding");
  std::string path = rootDir + "samples/" + std::to_string(idx) + ".mp3";
  if (std::filesystem::exists(path)) {
    std::filesystem::remove(path);
  }

  std::filesystem::path p = fname;
  if (p.extension() == ".mp3") {
    std::filesystem::copy(fname, path);
  } else {
    SF_INFO info_nonMp3, info_mp3;
    SNDFILE* nonMp3 = sf_open(fname.c_str(), SFM_READ, &info_nonMp3);
    info_mp3.samplerate = info_nonMp3.samplerate;
    info_mp3.channels = info_nonMp3.channels;
    info_mp3.format = SF_FORMAT_MPEG | SF_FORMAT_MPEG_LAYER_III;
    SNDFILE* mp3 = sf_open(path.c_str(), SFM_WRITE, &info_mp3);

    sf_count_t count;
    float* in = new float[1024];
    while (true) {
      count = sf_read_float(nonMp3, in, 1024);
      sf_write_float(mp3, in, 1024);
      if (count < 1024) break;
    }

    sf_close(nonMp3);
    sf_close(mp3);

    log(INFO, "Sample " + std::to_string(idx) + " - Override reformatted");
  }

  SF_INFO info;
  info.format = 0;
  SNDFILE* infile = sf_open(path.c_str(), SFM_READ, &info);
  if (info.samplerate != sampleRate || info.channels != 2) {
    sf_count_t count;
    SF_INFO infoR;
    infoR.channels = 2;
    infoR.format = info.format;
    infoR.samplerate = sampleRate;
    SNDFILE* outfile =
        sf_open((rootDir + "samples/" + std::to_string(idx) + "r.mp3").c_str(),
                SFM_WRITE, &infoR);
    double ratio = (double)sampleRate / info.samplerate;

    int err;
    SRC_STATE* src = src_new(SRC_SINC_FASTEST, info.channels, &err);
    SRC_DATA* dat = new SRC_DATA();
    int newNFrames = 1024 * ratio + 1;
    float *in = new float[1024 * info.channels],
          *out = new float[newNFrames * info.channels],
          *rechanneled = new float[newNFrames * 2];
    dat->data_in = in;
    dat->data_out = out;
    dat->input_frames = 1024;
    dat->output_frames = newNFrames;
    dat->src_ratio = ratio;

    while (true) {
      count = sf_read_float(infile, in, 1024 * info.channels);
      src_process(src, dat);

      if (info.channels != 2) {
        for (int i = 0; i < newNFrames; i++) {
          float sum = 0;
          for (int j = 0; j < info.channels; j++) {
            sum += out[info.channels * i + j];
          }
          rechanneled[2 * i] = rechanneled[2 * i + 1] = sum / info.channels;
        }

        sf_write_float(outfile, rechanneled, dat->output_frames_gen * 2);
      } else {
        sf_write_float(outfile, out, dat->output_frames_gen * 2);
      }
      if (count < 1024 * info.channels) break;
    }

    sf_close(outfile);
    sf_close(infile);
    src_delete(src);
    delete[] in;
    delete[] out;
    delete[] rechanneled;
    delete dat;

    std::filesystem::remove(path.c_str());
    std::filesystem::rename(
        (rootDir + "samples/" + std::to_string(idx) + "r.mp3").c_str(),
        path.c_str());

    log(INFO, "Sample " + std::to_string(idx) +
                  " - Override resampled & rechanneled");
  } else
    sf_close(infile);
}

void SoundboardManager::SetWaveform(WaveformViewer* wv) { this->wv = wv; }

void SoundboardManager::SetStreams(std::shared_ptr<Passthrough> passthrough,
                                   std::shared_ptr<Monitor> monitor,
                                   std::shared_ptr<Player> player) {
  log(INFO, "SetStreams() called");
  this->passthrough = passthrough;
  passthrough->SetPadding(settings["padding"].get<int>());
  this->monitor = monitor;
  monitor->SetSoundboardMonitorVol(settings["monitor"].get<int>() / 100.f);
  this->player = player;
  player->SetPadding(settings["padding"].get<int>());
}

void SoundboardManager::ResetStreams() {
  log(INFO, "ResetStreams() called");
  this->passthrough.reset();
  this->monitor.reset();
  this->player.reset();
}

void SoundboardManager::SetMonitoringVol(int n) {
  log(INFO, "Monitoring volume set to " + std::to_string(n) + "%");
  UpdateSettings<int>("monitor", n);
  monitor->SetSoundboardMonitorVol(settings["monitor"].get<int>() / 100.f);
}

void SoundboardManager::SetPadding(int ms) {
  log(INFO, "Padding set to " + std::to_string(ms) + "ms");
  UpdateSettings<int>("padding", ms);
  passthrough->SetPadding(ms);
  player->SetPadding(ms);
}
