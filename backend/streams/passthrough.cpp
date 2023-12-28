#include "passthrough.h"

/* The passthrough stream object, writes audio from the input device directly to
 * the output device. */

Passthrough::Passthrough(int inputDevice, int nInputChannels, int outputDevice, int nOutputChannels,
                         int sampleRate, int framesPerBuffer, std::string dir,
                         std::queue<float> *bufQueue)
    : AudioStream(inputDevice, nInputChannels, outputDevice, nOutputChannels, sampleRate, framesPerBuffer, dir,
                  "Passthrough") {
  initialSetup = false;

  this->data.bufQueue = bufQueue;

  // start a stream on creation of this object, since audio from the default
  // input should always be written to the virtual cable input
  this->err =
      Pa_OpenStream(&this->stream, input ? &this->inputParameters : NULL,
                    output ? &this->outputParameters : NULL, this->sampleRate,
                    this->framesPerBuffer, paClipOff,
                    input && output ? passthroughCallback : NULL, &this->data);
  streamSetup = true;
  if (this->err != paNoError) {
    done();
    return;
  }

  this->err = Pa_StartStream(this->stream);
  if (this->err != paNoError) {
    done();
    return;
  }

  data.nChannels = inputParameters.channelCount;

  data.rData = new recordData();
  data.rData->info = {};
  data.rData->info.samplerate = this->sampleRate;
  data.rData->info.channels = inputParameters.channelCount;
  data.rData->info.format = SF_FORMAT_MPEG | SF_FORMAT_MPEG_LAYER_III;

  data.pBuf = &pBuf;

  initialSetup = true;
  log(INFO, "Passthrough stream setup");
}

void Passthrough::Record(int idx) {
  if (!recording) {
    recording = true;

    std::string FILE_NAME = dir + "/samples/" + std::to_string(idx) + ".mp3";

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

    this->data.rData->inUse = true;
    log(INFO, "Key " + std::to_string(idx) + " - recording input");
  }
}

void Passthrough::Stop() {
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
  }
}

void Passthrough::SetPadding(int padding) {
  this->padding = padding;
  pBuf.setSize(this->inputParameters.channelCount * this->padding *
               (sampleRate / 1000));
  log(INFO, "Padding set to " + std::to_string(padding) + "ms");
}
