#pragma once

#include <JuceHeader.h>

#include "model/Glottis.h"
#include "model/Tract.h"
#include "model/Noise.h"

namespace model {

class VoiceProcessor final
{
public:
    struct ControlPoint
    {
        float tongueX{};
        float tongueY{};
        float constrictionX{};
        float constrictionY{};
        float tenseness{};
    };

    VoiceProcessor();

    void prepareToPlay(float sampleRate, int samplesPerBlock);
    void setControlPoint(const ControlPoint& cp);
    void trigger(const VoiceProcessor::ControlPoint& cp);
    void retrigger(const VoiceProcessor::ControlPoint& cp);
    void release();
    void process(float* out, int numFrames);

    void setFrequency(float f, bool force = false);
    void setVibrato(float level);
    void setTenseness(float t);

private:

    void update();
    void updateControlPoint();

    Glottis glottis{};
    Tract tract{};
    WhiteNoise whiteNoise{};

    juce::IIRFilter fricativeFilter{};
    juce::IIRFilter aspirateFilter{};

    ControlPoint targetControlPoint{};
    float timePerBlock{};

    float tongueX{};
    float tongueY{};
    float constrictionX{ 0.9f };
    float constrictionY{ 0.9f };

    float fricativeIntensity{ 0.0f };
    bool constrictionActive{ true };
};

} // namespace model
