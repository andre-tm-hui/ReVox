#include "player.h"

/* The player object, dedicated to loading a file and playing it to a specified
 * output device. */

Player::Player(device inputDevice, device outputDevice, int sampleRate,
               int framesPerBuffer, std::string dir,
               std::queue<float>* bufQueue)
    : AudioStream(inputDevice, outputDevice, sampleRate, framesPerBuffer, dir,
                  "Player") {
  initialSetup = false;

  data.bufQueue = bufQueue;

  data.rData = new recordData();
  data.rData->info = {};
  data.rData->info.samplerate = this->sampleRate;
  data.rData->info.channels = inputParameters.channelCount;
  data.rData->info.format = SF_FORMAT_MPEG | SF_FORMAT_MPEG_LAYER_III;
  data.rData->inUse = false;
  data.pBuf = &pBuf;

  files = {};
  data.timers = {};
  data.queue = new std::vector<SNDFILE*>();

  // written files are set to always have 2 channels
  SF_INFO info;
  info.channels = 2;
  info.samplerate = this->sampleRate;
  data.info = info;

  // open a stream, as player is expected to always be ready to play something
  err = Pa_OpenStream(&stream, input ? &this->inputParameters : NULL,
                      output ? &this->outputParameters : NULL, this->sampleRate,
                      this->framesPerBuffer, paClipOff,
                      output ? playCallback : NULL, &this->data);
  if (err != paNoError) {
    done();
    return;
  }
  streamSetup = true;

  this->err = Pa_StartStream(this->stream);
  if (this->err != paNoError) {
    done();
  }
  initialSetup = true;

  log(INFO, "Player stream setup");
}

void Player::Play(int idx, json settings) {
  log(INFO, "Key " + std::to_string(idx) + " - Adding to queue");
  // check if a file corresponding to the keycode exists
  std::string FILE_NAME = dir + "/samples/" + std::to_string(idx) + ".mp3";
  if (!std::filesystem::exists(FILE_NAME.c_str())) {
    log(WARN, "File doesn't exist");
    return;
  }

  // add a new list if the hotkey hasn't been played yet
  if (files.find(idx) == files.end()) {
    files[idx] = {};
  }

  // check if the hotkey limit is reached
  if (settings["maxCopies"].get<int>() != -1 &&
      files[idx].size() >= settings["maxCopies"]) {
    // Cleanup first item of data.files[keycode]
    SNDFILE* clear = files[idx].front();
    files[idx].pop();
    if (data.timers.contains(clear)) {
      data.timers[clear].timeLeft = -1;
    }
  }

  // open the file and add it to the queue
  SNDFILE* file = sf_open(FILE_NAME.c_str(), SFM_READ, &data.info);
  if (settings["endAt"] == -1) settings["endAt"] = data.info.frames;
  int startAt = settings["startAt"].get<int>(), readRemaining = startAt,
      readN = 1;
  while (readN * 2 < startAt && readN < 1024) readN *= 2;

  sf_count_t count;
  while (1) {
    float* temp = new float[2 * readN];
    count = sf_read_float(file, temp, 2 * readN);
    readRemaining -= readN;
    if (count < 2 * readN || readRemaining <= 0) break;
    while (readN > readRemaining && readN > 1) readN /= 2;
    delete[] temp;
  }

  files[idx].push(file);
  data.timers[file].timeLeft = settings["endAt"].get<int>() - startAt;
  data.timers[file].volume = settings["volume"];

  log(INFO, "Key " + std::to_string(idx) + " - Added to queue");
}

/* Utility function to check if a file corresponding to the keycode exists */
bool Player::CanPlay(int idx) {
  std::string FILE_NAME = dir + "samples/" + std::to_string(idx) + ".mp3";
  return std::filesystem::exists(FILE_NAME.c_str());
}

void Player::StopAll() {
  for (auto const& [file, time] : data.timers) {
    sf_close(file);
  }
  data.timers = {};
  files = {};
  log(INFO, "All sounds stopped");
}

/* Utility function to rename a file if the keybind is being rebinded */
void Player::Rename(int idxFrom, int idxTo) {
  std::filesystem::rename(dir + "samples/" + std::to_string(idxFrom) + ".mp3",
                          dir + "samples/" + std::to_string(idxTo) + ".mp3");
}

void Player::Record(int idx) {
  if (!recording) {
    recording = true;

    // create a filename based on the keycode
    std::string FILE_NAME = dir + "/samples/" + std::to_string(idx) + "m.mp3";

    // set the sndfile info to be a .mp3 file
    this->data.rData->info.samplerate = this->sampleRate;
    this->data.rData->info.channels = 2;
    this->data.rData->info.format = SF_FORMAT_MPEG | SF_FORMAT_MPEG_LAYER_III;

    // open the file
    this->data.rData->file =
        sf_open(FILE_NAME.c_str(), SFM_WRITE, &this->data.rData->info);
    if (sf_error(this->data.rData->file) != SF_ERR_NO_ERROR) {
      log(ERR,
          "Libsndfile error: " + std::string(sf_strerror(data.rData->file)));
      return;
    }

    if (pBuf.len() > 0) {
      log(INFO, "Writing pre-recorded buffer");
      sf_write_float(this->data.rData->file, pBuf.get(), pBuf.len());
    }

    this->idx = idx;
    this->data.rData->inUse = true;
    log(INFO, "Key " + std::to_string(idx) + " - recording system");
  }
}

void Player::Stop() {
  // only call if currently recording
  if (recording) {
    // tell the callback to stop recording, and wait for thee callback to finish
    // up any work
    this->data.rData->inUse = false;
    Sleep(200);
    // close the file being written to
    sf_close(data.rData->file);

    recording = false;
    log(INFO, "Recording stopped");

    Merge();
  }
}

void Player::Merge() {
  if (!std::filesystem::exists(
          (dir + "samples/" + std::to_string(idx) + ".mp3").c_str())) {
    std::filesystem::rename(
        (dir + "samples/" + std::to_string(idx) + "m.mp3").c_str(),
        (dir + "samples/" + std::to_string(idx) + ".mp3").c_str());
    return;
  }

  log(INFO, "Merging input and system recordings");
  // load both recorded files, add them together and output to a new file
  SNDFILE *fileA, *fileB, *fileOut;
  float bufferA[1024], bufferB[1024], bufferOut[1024];
  SF_INFO infoA = data.rData->info, infoB = data.rData->info;
  infoA.format = 0;

  fileA = sf_open((dir + "samples/" + std::to_string(idx) + "m.mp3").c_str(),
                  SFM_READ, &infoA);

  fileB = sf_open((dir + "samples/" + std::to_string(idx) + ".mp3").c_str(),
                  SFM_READ, &infoB);

  fileOut =
      sf_open((dir + "samples/" + std::to_string(idx) + "out.mp3").c_str(),
              SFM_WRITE, &data.rData->info);

  sf_count_t countA, countB;

  while (1) {
    countA = sf_read_float(fileA, bufferA, 1024);
    countB = sf_read_float(fileB, bufferB, 1024);

    if (countA == 0 && countB == 0) break;

    // combine the two buffers and write to the output file
    for (int i = 0; i < (countA > countB ? countA : countB); i++) {
      float a = i < countA ? bufferA[i] : 0;
      float b = i < countB ? bufferB[i] : 0;
      bufferOut[i] = 0.5 * (a + b);
    }
    sf_write_float(fileOut, bufferOut, 1024);

    // break the while loop if both files reach EOF
    if (countA < 1024 && countB < 1024) {
      break;
    }
  }

  sf_close(fileA);
  sf_close(fileB);
  sf_close(fileOut);

  // cleanup
  if (std::filesystem::exists(dir + "/samples/" + std::to_string(idx) +
                              "out.mp3")) {
    std::filesystem::remove(
        (dir + "/samples/" + std::to_string(idx) + ".mp3").c_str());
    std::filesystem::remove(
        (dir + "/samples/" + std::to_string(idx) + "m.mp3").c_str());
    std::filesystem::rename(
        (dir + "/samples/" + std::to_string(idx) + "out.mp3").c_str(),
        (dir + "/samples/" + std::to_string(idx) + ".mp3").c_str());
  }
  log(INFO, "Merged");
}

void Player::SetPadding(int padding) {
  this->padding = padding;
  pBuf.setSize(this->inputParameters.channelCount * this->padding *
               (sampleRate / 1000));
  log(INFO, "Padding set to " + std::to_string(padding) + "ms");
}
