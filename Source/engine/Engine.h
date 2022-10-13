#pragma once

#include <JuceHeader.h>
#include <bitset>
#include "core/Queue.h"
#include "engine/Interpolator.h"
#include "engine/Voice.h"
#include "engine/Lyrics.h"

namespace engine {

class Engine final
{
public:

    constexpr static float INTERNAL_SAMPLE_RATE = 44100.0f;
    constexpr static float INTERNAL_SAMPLE_RATE_R = 1.0f / INTERNAL_SAMPLE_RATE;

    constexpr static size_t NUM_CHANNELS = 2;

    constexpr static size_t SUB_FRAME_LENGTH = 32;

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

    size_t getNumPhares() const { return lyricsNumPhrases.load(); }
    size_t getCurrentPhraseIndex() const { return phraseIndex.load(); }
    Lyrics::Phrase getCurrentPhrase() const;

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

    VoicePool voicePool;
    core::List<Voice> activeVoices{};

    float externalSampleRate{ 44100.0f };

    std::atomic<bool> legato{ true };

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

    Interpolator interpolator{1.0f, NUM_CHANNELS};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Engine)
};

} // namespace engine
