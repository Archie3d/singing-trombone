#pragma once

#include <JuceHeader.h>
#include "core/List.h"
#include "model/VoiceProcessor.h"
#include "engine/Envelope.h"
#include <vector>
#include <atomic>

namespace engine {

class Engine;

class Voice final : public core::ListItem<Voice>
{
public:

    /** A single phonene defined by its symbol and play duration. */
    struct Phoneme
    {
        char symbol{};
        float duration{ 0.1f }; // [s]
    };

    /**
     * A single phrase.
     *
     * A phrase is constructed of two parts - attack and release.
     * Each phase is represented by a set of phonemes. The attack phrase will
     * be vocalised on note trigger. The last phoneme of the attack phrase will be sustained
     * while the note is sustained. On note release the release phrase will be vocalised.
     * The very last phoneme of the release phrase will trigger envelope release, and will be
     * released together with the envelope.
     */
    struct Phrase
    {
        constexpr static size_t MAX_PHONEMES_PER_PHRASE = 8;

        std::array<Phoneme, MAX_PHONEMES_PER_PHRASE> attack{};
        size_t numAttackPhonemes{};

        std::array<Phoneme, MAX_PHONEMES_PER_PHRASE> release{};
        size_t numReleasePhonemes{};

        float tenseness{ 0.6f };

        void parse(const String& a, const String& r);
    };

    /**
     * Voice trigger record.
     */
    struct Trigger
    {
        int key{};
        float velocity{};
        Envelope::Spec envelope{};
        Phrase phrase{};
    };

    Voice() = delete;
    Voice(Engine& eng);

    void prepareToPlay(float sampleRate, int samplesPerBlock);
    void trigger(const Trigger& t);
    void retrigger(const Trigger& t);
    const Trigger& getTriggerRecord() const { return triggerRecord; }
    void release();
    void process(float* outL, float* outR, size_t numFrames);
    bool isReleasing() const;
    bool isOver() const;

    void reset();

private:

    Engine& engine;
    Trigger triggerRecord{};
    Envelope envelope{};

    model::VoiceProcessor voiceProcessor{};

    bool attackPhase{};
    size_t phonemeIndex{};
    size_t generatedSamplesInPhoneme{};
    size_t totalSamplesInPhoneme{};

    float vibratoLevel{};
};

//==============================================================================

class VoicePool final
{
public:
    constexpr static size_t defaultMaxVoices = 32;

    VoicePool(Engine& eng, size_t numVoices = defaultMaxVoices);

    void prepareToPlay(float sampleRate, int samplesPerBlock);
    Voice* trigger(const Voice::Trigger& triger);
    void recycle(Voice* voice);

    int getVoiceCount() const { return voiceCount.load(); }

private:
    Engine& engine;
    std::vector<Voice> voices;
    core::List<Voice> idleVoices;
    std::atomic<int> voiceCount;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VoicePool)
};

} // namespace engine
