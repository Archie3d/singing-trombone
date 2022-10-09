#pragma once

#include <JuceHeader.h>

namespace model {

class Glottis
{
    Glottis();

    void reset();
    void prepareToPlay(float sampleRate);
    float tick(float lambda, float noise);

private:
    void initWaveform(float lambda = 0.0f);
    float normalizedLFWaveform(float t);

    float sampleRate_r{};
    float timeInWaveform{};
    float waveformLength{};
};

} // namespace model
