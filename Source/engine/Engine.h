#pragma once

#include <JuceHeader.h>
#include <bitset>
#include "engine/Voice.h"

namespace engine {

class Engine final
{
public:
    Engine();

    void prepareToPlay(float sampleRate, int samplesPerBlock);

    void process(float* outL, float* outR, size_t numFrames);
    void processMidiMessage(const MidiMessage& msg);

    float getSampleRate() const { return sampleRate; }

    int getVoiceCount() const { return voicePool.getVoiceCount(); }

private:

    void updateParameters(size_t numFrames);
    void noteOn(const MidiMessage& msg);
    void noteOff(const MidiMessage& msg);
    void controlChange(const MidiMessage& msg);
    void releaseSustainedVoices();

    VoicePool voicePool;
    core::List<Voice> activeVoices{};

    float sampleRate{ 44100.0f };

    AudioBuffer<float> mixBuffer{};

    std::bitset<128> keysState{};
    bool sustained{};    

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Engine)
};

} // namespace engine
