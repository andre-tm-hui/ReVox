#include "pitchshifter.h"

PitchShifter::PitchShifter(int bufSize, int inputSize, int sampleRate)
    : bufSize(bufSize),
      inputSize(inputSize),
      sampleRate(sampleRate)
{
    freq = new Frequency(bufSize);

    input.setSize(3 * bufSize);
    output = new float[3 * bufSize];
    memset(output, 0, sizeof(float) * 3 * bufSize);
}

void PitchShifter::process(float *buf) {
    // write the new frame to the input buffer
    input.write(buf, inputSize);
    // copy the corresponding output frame to buf
    memcpy(buf, output + framesWritten, sizeof(float) * inputSize);
    // set the copied data to 0
    //memset(output + outputPos + framesWritten, 0, sizeof(float) * inputSize);
    // increment the framesWritten
    framesWritten += inputSize;
    if (framesWritten >= bufSize)
    {
        auto time = std::chrono::steady_clock::now().time_since_epoch();
        framesWritten %= bufSize;
        std::thread t(&PitchShifter::processRepitch, this, time);
        t.detach();
    }
}

void PitchShifter::processRepitch(std::chrono::nanoseconds startTime) {
    // move the output start position pointer
    //outputPos += bufSize;
    //outputPos %= 3 * bufSize;
    memcpy(output, output + bufSize, sizeof(float) * bufSize * 2);
    memset(output + bufSize * 2, 0, sizeof(float) * bufSize);
    // get a copy of the input buffer, temporally ordered
    float *tempInput = input.get();

    float factor = 1.f;
    // in a 3*bufSize array, we work on the middle bufSize, ensuring we have padding around it when we extract windows
    float period = freq->GetPeriod(tempInput + bufSize);

    auto futureMarkers = std::async(std::launch::async, &PitchShifter::getMarkers, this, period, tempInput + bufSize);

    // Check if the period is outside of human frequency ranges
    if (period > 800 || period < 24)  {
        setupFlag = false;
        lastMarker = (float)bufSize+1.f;
        cleanup(factor, period);
        memcpy(output + bufSize, tempInput + bufSize, sizeof(float) * bufSize);
        return;
    }

    // repitch if enabled
    if (repitch) factor *= repitchFactor;

    // autotune if enabled
    if (true) {
        float ffreq = (float)sampleRate / period; // fundamental frequency
        float targetSemitone = getTargetFreqFactor(ffreq);
        factor *= (C_ONE * pow(2.f, targetSemitone / 12.f)) / ffreq;
    }
    //factor *= 2.f;

    // inverse the factor before applying to the marker spacing
    factor = 1.f / factor;

    // get period markers for the current buffer
    std::vector<int> markers = futureMarkers.get();
    if (markers.size() <= 0) {
        cleanup(factor, period); return;
    }

    // Get windows on another thread
    auto futureWindows = std::async(std::launch::async, &PitchShifter::getWindows, this, tempInput + bufSize, period, markers, factor);

    // Get synthesized markers on main thread
    std::vector<int> synthesizedMarkers = synthesizeMarkers(period, factor, markers);
    if (synthesizedMarkers.size() <= 0) {
        cleanup(factor, period); return;
    }
    lastMarker = synthesizedMarkers.back();

    // Wait for windows to be gotten
    std::vector<std::vector<float>> windows = futureWindows.get();
    if (windows.size() <= 0) {
        cleanup(factor, period); return;
    }

    // overlap-add windows to output buffer
    overlapAdd(windows, markers, synthesizedMarkers);
    setupFlag = true;

    // update carry-over fields for future repitching
    cleanup(factor, period);
    //std::cout << "Processing Time: " << std::chrono::steady_clock::now().time_since_epoch() - startTime << std::endl;
}


float PitchShifter::getTargetFreqFactor(float ffreq) {
    // Setup some base values
    // calculate the maximum jump - if retune speed is 0ms, jump can be 1 full semitone.
    float maxJump = retuneSpeed > 0 ? ((float)bufSize / (float)sampleRate) / ((float)retuneSpeed / 1000.f) : 13;
    float semitoneOffset = 12 * log(ffreq / C_ONE) / log(2.f); // convert fundamental frequency into semitones from C1
    float scaleOffset = fmod(semitoneOffset, 12); // bring it in range 0-12
    int octaveOffset = (int)floor(semitoneOffset / 12.f); // get the octave of the note
    int closest = 0;

    // Use 13 to check for rounding up to C instead of down
    for (int i = 0; i < 13; i++) {
        if (validNotes[i % 12]) {
            if (scaleOffset - i < 0) {
                closest = i - scaleOffset > scaleOffset - closest ? closest : i;
                break;
            }
            closest = i;
        }
    }

    // Adjust target with respects to retune speed
    float targetSemitone = closest + octaveOffset * 12;
    if (abs(semitoneOffset - targetSemitone) > abs(lastTargetSemitone - targetSemitone))
        semitoneOffset = lastTargetSemitone;
    if (abs(semitoneOffset - targetSemitone) > maxJump)
        targetSemitone = semitoneOffset + (semitoneOffset < targetSemitone ? maxJump : -maxJump);

    lastTargetSemitone = targetSemitone;
    // Return the target semitone
    return targetSemitone;
}

std::vector<int> PitchShifter::getMarkers(float period, float *input) {
    // find the first peak and affix the first marker
    float max = -INFINITY;
    int marker = 0;
    for (int i = 0; i < (int)period; i++)
    {
        if (input[i] > max)
        {
            max = input[i];
            marker = i;
        }
    }

    // Get all the "markers", i.e. peaks of the input signal
    std::vector<int> markers = { marker };
    for (float m = marker; m < bufSize; m+=period) markers.push_back((int)round(m));
    /*for (float m = marker + period; m < bufSize; m += period) {
        float max = 0.f;
        int newMarker = 0, periodCenter = round(m);
        for (int i = -period/4; i < period/4; i++) {
            if (input[periodCenter + i] > max) {
                max = input[periodCenter + i];
                newMarker = periodCenter + i;
            }
        }
        markers.push_back(newMarker);
    }*/
    return markers;
}

std::vector<int> PitchShifter::synthesizeMarkers(float period, float factor, std::vector<int> markers) {
    // Get synthesized markers
    if (markers.size() == 0) return {};

    float marker_s = markers[0];
    // get the first marker, based on where the last marker is
    if (setupFlag && lastMarker < bufSize)
        //marker_s = lastMarker - bufSize + period * factor;
        marker_s = (1.f - ((bufSize - lastMarker) / (prevPeriod * prevFactor))) * period * factor;

    // generate the list of synthesized markers
    std::vector<int> markers_s;
    for (float m = marker_s; m < bufSize; m += period * factor)
        markers_s.push_back((int)round(m));

    // if it's empty, escape to a fallback
    if (markers_s.empty()) return {};

    return markers_s;
}

std::vector<std::vector<float>> PitchShifter::getWindows(float *buf, float period, std::vector<int> markers, float factor)
{
    std::vector<std::vector<float>> windows;
    std::vector<std::future<std::vector<float>>> futures;
    for (auto marker : markers)
        futures.push_back(std::async(&PitchShifter::getWindow, this, buf, period, marker, factor));

    for (auto& future : futures)
        windows.push_back(future.get());

    return windows;
}

std::vector<float> PitchShifter::getWindow(float *buf, float period, int marker, float factor) {
    std::vector<float> window(2*period, 0);
    for (int i = -period; i < period; i++)
        window[i+period] = buf[(int)marker+i];
    window = Window::Hann(window);

    //if (factor != 1.f) window = resample(window, factor);
    return window;
}

std::vector<float> PitchShifter::resample(std::vector<float> input, float factor)
{
    std::vector<float> v(ceil(factor * input.size()), 0.f);
    float i_scaled;
    for (int i = 0; i < v.size(); i++) {
        i_scaled = (float)i / (v.size()-1) * (input.size()-1);
        v[i] = std::lerp(input[floor(i_scaled)],
                         input[ceil(i_scaled)],
                         i_scaled - floor(i_scaled));
    }
    return v;
}

void PitchShifter::overlapAdd(std::vector<std::vector<float>> windows, std::vector<int> markers, std::vector<int> synthesizedMarkers) {
    int markerIdx = 0;
    std::vector<std::thread> threads;
    for (auto synthesizedMarker : synthesizedMarkers) {
        // Find the window closest to the marker
        while (markerIdx < markers.size() - 1 && markers[markerIdx] < synthesizedMarker)
            markerIdx++;
        if (markerIdx > 0 && markers[markerIdx] - synthesizedMarker > synthesizedMarker - markers[markerIdx - 1])
            markerIdx--;

        std::thread t(&PitchShifter::addWindow, this, windows[markerIdx], synthesizedMarker);
        threads.push_back(std::move(t));
    }
    for (auto& t : threads)
        t.join();
}

void PitchShifter::addWindow(std::vector<float> window, int marker) {
    window = Window::Hann(window);
    //int currentPos = outputPos + bufSize + marker - window.size() / 2;
    int currentPos = bufSize + marker - window.size() / 2;
    for (int i = 0; i < window.size(); i++, currentPos++) {
        if (currentPos < 0)
            currentPos += bufSize * 3;
        else if (currentPos > bufSize * 3)
            currentPos -= bufSize * 3;

        output[currentPos] += window[i];
    }
}

void PitchShifter::reset() {
    for (int i = 0; i < bufSize * 3; i++) {
        input[i] = 0.f;
        output[i] = 0.f;
    }
}

void PitchShifter::cleanup(float factor, float period) {
    prevFactor = factor;
    prevPeriod = period;
}
