#pragma once

#include <JuceHeader.h>

#include "Noise.h"

namespace model {

class Glottis
{
public:
    Glottis();

    void reset();
    void prepareToPlay(float sampleRate);
    float tick(float lambda, float noise);
	float getNoiseModulator() const;
    void finishBlock();

    void setFrequency(float f, bool force = false);
    void setTenseness(float t);

private:
    void initWaveform(float lambda = 0.0f);
    float normalizedLFWaveform(float t);

	SimplexNoise simplexNoise{};

    float sampleRate_r{ 1.0f / 44100.0f };
    float timeInWaveform{};
    float waveformLength{};

    constexpr static float defaultFrequency = 140.0f;

    float frequency{ defaultFrequency };
    float oldFrequency{ defaultFrequency };
    float newFrequency{ defaultFrequency };
    float smoothFrequency{ defaultFrequency };
    float targetFrequency{ defaultFrequency };

    constexpr static float defaultTenseness = 0.6f;

    float oldTenseness{ defaultTenseness };
    float newTenseness{ defaultTenseness };
    float targetTenseness{ defaultTenseness };

	float alpha{};
	float E0{};
	float epsilon{};
	float shift{};
	float delta{};
	float Te{};
	float omega{};

	float totalTime{};
	float intensity{};
    float loudness{ 1.0f };
	float vibratoAmount{};
	float vibratoFrequency{ 6.0f };
	bool autoWobble{ true };
	bool isTouched{ false };
	bool alwaysVoice{ true };
};

} // namespace model
