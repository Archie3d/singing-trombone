#pragma once

#include <JuceHeader.h>

#include "Noise.h"

namespace model {

class Glottis
{
public:
    Glottis();

    void reset();
    void prepareToPlay(float sampleRate, float timePerBlock);
    float tick(float lambda, float noise);
	float getNoiseModulator() const;
    void finishBlock();

    void setFrequency(float f, bool force = false);
    void setTenseness(float t);

    void setTouched(bool b) { isTouched = b; }
    void setVibrato(float v) { vibratoAmount = jlimit(0.0f, 1.0f, v); }

private:
    void initWaveform(float lambda = 0.0f);
    float normalizedLFWaveform(float t);

	SimplexNoise simplexNoise{};

    float sampleRate_r{ 1.0f / 44100.0f };
    float smoothRate{ 1.0f };
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
	bool autoWobble{ false };
	bool isTouched{ false };
	bool alwaysVoice{ false };
};

} // namespace model
