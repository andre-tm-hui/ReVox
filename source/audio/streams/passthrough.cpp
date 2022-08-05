#include "passthrough.h"

/* The passthrough stream object, writes audio from the input device directly to the output device. */

Passthrough::Passthrough(device inputDevice, device outputDevice, int sampleRate, int framesPerBuffer, std::string dir, float *inputBuffer)
    : AudioStream(inputDevice, outputDevice, sampleRate, framesPerBuffer, dir)
{
    initialSetup = false;

    this->data.buf = inputBuffer;

    // start a stream on creation of this object, since audio from the default input should always be written to the virtual cable input
    this->err = Pa_OpenStream(
                &this->stream,
                input ? &this->inputParameters : NULL,
                output ? &this->outputParameters : NULL,
                this->sampleRate,
                this->framesPerBuffer,
                paClipOff,
                input && output ? passthroughCallback : NULL,
                &this->data
                );
    streamSetup = true;
    if (this->err != paNoError)
    {
        std::cout<<Pa_GetErrorText(this->err)<<std::endl;
        done(); return;
    }

    this->err = Pa_StartStream(this->stream);
    if (this->err != paNoError)
    {
        std::cout<<Pa_GetErrorText(this->err)<<std::endl;
        done(); return;
    }

    data.nChannels = inputParameters.channelCount;

    data.reverb = new revmodel();

    data.pitchShift = new PitchShift(framesPerBuffer, sampleRate);

    data.rData = new recordData();
    data.rData->info = {};
    initialSetup = true;
}

void Passthrough::SetFX(json settings)
{
    data.reverb->setEnabled(settings["reverb"]["enabled"].get<bool>());
    (*checkboxes)["reverb"]->setCheckState(data.reverb->getEnabled() ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    data.reverb->setwet(settings["reverb"]["mix"].get<float>());
    data.reverb->setdry(1.f - settings["reverb"]["mix"].get<float>());
    data.reverb->setdamp(settings["reverb"]["damp"].get<float>());
    data.reverb->setroomsize(settings["reverb"]["roomsize"].get<float>());
    data.reverb->setwidth(settings["reverb"]["width"].get<float>());

    data.pitchShift->setAutotune(settings["autotune"]["enabled"].get<bool>());
    (*checkboxes)["autotune"]->setCheckState(data.pitchShift->getAutotune() ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    //data.ps.setSpeed(settings["autotune"]["speed"].get<float>();

    data.pitchShift->setPitchshift(settings["pitch"]["enabled"].get<bool>());
    (*checkboxes)["pitchshift"]->setCheckState(data.pitchShift->getPitchshift() ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    data.pitchShift->setPitchscale(settings["pitch"]["pitch"].get<float>());
}


void Passthrough::Record(int keycode)
{
    if (!recording)
    {
        std::string FILE_NAME = dir + "/samples/" + std::to_string(keycode) + ".mp3";

        // set the sndfile info to be a .mp3 file
        this->data.rData->info.samplerate = this->sampleRate;
        this->data.rData->info.channels = 2;
        this->data.rData->info.format = SF_FORMAT_MPEG | SF_FORMAT_MPEG_LAYER_III;

        // open the file
        this->data.rData->file = sf_open(FILE_NAME.c_str(), SFM_WRITE, &this->data.rData->info);
        if (sf_error(this->data.rData->file) != SF_ERR_NO_ERROR) {
            fprintf(stderr, (sf_strerror(data.rData->file)));
            return;
        }

        this->data.rData->inUse = true;

        recording = true;
    }
}

void Passthrough::Stop(int keycode)
{
    // only call if currently recording
    if (recording) {
        // tell the callback to stop recording, and wait for thee callback to finish up any work
        this->data.rData->inUse = false;
        Sleep(200);
        // close the file being written to
        sf_close(data.rData->file);

        recording = false;
    }
}
