#pragma once

#include <JuceHeader.h>

namespace engine {

/** ADSR envelope.
 
    ADSR envelope with exponential attack, decay, and release. 
*/
class Envelope final
{
public:

    enum class State
    {
        Off,
        Attack,
        Decay,
        Sustain,
        Release,
        NumStates
    };

    constexpr static float attackTargetRatio{ 0.3f };
    constexpr static float decayReleaseTargetRatio{ 0.0001f };
    constexpr static float defaultSampleRate{ 44100.0f };

    struct Spec
    {
        float attack    { 0.01f };
        float decay     { 0.0f };
        float sustain   { 1.0f }; 
        float release   { 0.1f };
        float sampleRate{ defaultSampleRate };
    };

    Envelope();

    State getState() const noexcept { return currentState; }

    void trigger(const Spec& spec);
    void retrigger();
    void release();
    void release(float t);

    float getNext();

    float getLevel() const noexcept { return currentLevel; }

private:

    static float calculate(float rate, float targetRatio);

    State currentState{ State::Off };
    float currentLevel{ 0.0f };

    float attackRate  { 0.0f };
    float attackCoef  { 0.0f };
    float attackBase  { 0.0f };

    float decayRate   { 0.0f };
    float decayCoef   { 0.0f };
    float decayBase   { 0.0f };

    float releaseRate { 0.0f };
    float releaseCoef { 0.0f };
    float releaseBase { 0.0f };

    float sustainLevel{ 0.0f };

    float sampleRate  { defaultSampleRate };
};

} // namespace engine
