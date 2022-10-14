#pragma once

#include <JuceHeader.h>
#include <bitset>
#include "core/Queue.h"
#include "engine/Interpolator.h"
#include "engine/Parameter.h"
#include "engine/Voice.h"
#include "engine/Lyrics.h"

namespace engine {

class Engine final
{
public:

    /**
     * All internal processing is performed at this sample rate.
     * The vocal model has been tuned originally for 44100Hz, so we cannot
     * use anything else without retuning the model. 
     */
    constexpr static float INTERNAL_SAMPLE_RATE = 44100.0f;
    constexpr static float INTERNAL_SAMPLE_RATE_R = 1.0f / INTERNAL_SAMPLE_RATE;

    /**
     * The vocal model produces monophonic audio. However individual voice
     * processing is in stereo, in case there is any effect applied to the voice. 
     */
    constexpr static size_t NUM_CHANNELS = 2;

    /**
     * Processing is subdivided into smaller frames that get passed
     * via the interpolator to match the internal smaple rate with the one
     * imposed by the host.
     */
    constexpr static size_t SUB_FRAME_LENGTH = 32;

    enum Param
    {
        PARAM_VOLUME,
        PARAM_EXPRESSION,
        PARAM_VIBRATO,

        TOTAL_PARAMETERS
    };

    Engine();

    void prepareToPlay(float sampleRate, int samplesPerBlock);

    void process(float* outL, float* outR, size_t numFrames);
    void processMidiMessage(const MidiMessage& msg);
    void processLyrics();

    float getExternalSampleRate() const { return externalSampleRate; }

    int getVoiceCount() const { return voicePool.getVoiceCount(); }

    Result setLyrics(const String& str);

    void rewind();

    void setLegato(bool l) { legato = l; }
    bool isLegato() const { return legato.load(); }
    void setVibrato(float v) { parameters[PARAM_VIBRATO].setValue(v); }
    float getVibrato() const { return parameters[PARAM_VIBRATO].getTargetValue(); }
    void setVolume(float v) { parameters[PARAM_VOLUME].setValue(v); }
    float getVolume() const { return parameters[PARAM_VOLUME].getTargetValue(); }
    void setExpression(float v) { parameters[PARAM_EXPRESSION].setValue(v); }
    float getExpression() const { return parameters[PARAM_EXPRESSION].getTargetValue(); }
    void setEnvelopeAttack(float v) { envelopeAttack = jlimit(0.0f, 10.0f, v); }
    float getEnvelopeAttack() const { return envelopeAttack; }
    void setEnvelopeDecay(float v) { envelopeDecay = jlimit(0.0f, 10.0f, v); }
    float getEnvelopeDecay() const { return envelopeDecay; }
    void setEnvelopeSustain(float v) { envelopeSustain = jlimit(0.0f, 1.0f, v); }
    float getEnvelopeSustain() const { return envelopeSustain; }
    void setEnvelopeRelease(float v) { envelopeRelease = jlimit(0.0f, 10.0f, v); }
    float getEnvelopeRelease() const { return envelopeRelease; }

    size_t getNumPhrases() const { return lyricsNumPhrases.load(); }
    size_t getCurrentPhraseIndex() const { return phraseIndex.load(); }
    const Lyrics::Phrase& getCurrentPhrase() const;

    const ParameterPool& getParameters() const { return parameters; }

    /**
     * This method must be called periodically outside of the audio thread.
     * It will release disposable objects, if any.
     */
    void performHousekeeping();

private:

    Result setLyrics(const Lyrics::Ptr& ptr);

    void updateParameters(size_t numFrames);
    void noteOn(const MidiMessage& msg);
    void noteOff(const MidiMessage& msg);
    void controlChange(const MidiMessage& msg);
    void releaseSustainedVoices();

    void processSubFrame();

    float externalSampleRate{ 44100.0f };

    VoicePool voicePool;
    core::List<Voice> activeVoices{};

    ParameterPool parameters{ TOTAL_PARAMETERS };

    /* Voice static parameters (these are not smoothed once voice has been triggered) */
    std::atomic<bool> legato{ true };
    std::atomic<float> envelopeAttack{ 0.3f };
    std::atomic<float> envelopeDecay{ 0.1f };
    std::atomic<float> envelopeSustain{ 0.75f };
    std::atomic<float> envelopeRelease{ 0.3f };

    AudioBuffer<float> subFrameBuffer{ NUM_CHANNELS, SUB_FRAME_LENGTH };
    AudioBuffer<float> mixBuffer{ NUM_CHANNELS, SUB_FRAME_LENGTH };
    size_t remainedSamples{};

    std::bitset<128> keysState{};
    bool sustained{};

    constexpr static size_t lyricsQueueSize = 64;
    core::Queue<Lyrics::Ptr, lyricsQueueSize> setLyricsQueue{};
    core::Queue<Lyrics::Ptr, lyricsQueueSize> disposeLyricsQueue{};

    constexpr static size_t lyricsMaxPhrases = 4096;
    std::array<Voice::Phrase, lyricsMaxPhrases> lyrics{};
    std::atomic<size_t> lyricsNumPhrases{};
    std::atomic<size_t> phraseIndex{};

    Lyrics::Ptr cachedLyrics{};

    Interpolator interpolator{ 1.0f, NUM_CHANNELS };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Engine)
};

} // namespace engine
