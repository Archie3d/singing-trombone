#include "engine/Voice.h"
#include "engine/Engine.h"
#include <map>

namespace engine {

const static std::map<char, model::VoiceProcessor::ControlPoint> phonemes {
    { 'a', { 0.20f, 0.19f, 0.80f, 0.00f, 0.60f } },
    { 'i', { 1.00f, 0.05f, 0.76f, 0.68f, 0.60f } },
    { 'y', { 1.00f, 0.20f, 0.50f, 0.70f, 0.60f } },
    { 'u', { 0.60f, 0.00f, 0.91f, 0.68f, 0.60f } },
    { 'e', { 0.55f, 0.80f, 0.00f, 0.00f, 0.60f } },
    { 'o', { 0.00f, 0.00f, 0.86f, 0.85f, 0.60f } },
    { 'l', { 0.20f, 0.19f, 0.82f, 0.70f, 0.60f } },
    { 's', { 0.87f, 0.22f, 0.83f, 0.64f, 0.00f } },
    { 'z', { 0.87f, 0.22f, 0.83f, 0.63f, 0.87f } },
    { 'k', { 0.00f, 0.00f, 0.21f, 0.09f, 0.60f } },
    { 'q', { 0.87f, 0.76f, 0.21f, 0.09f, 0.33f } },
    { 'g', { 0.00f, 0.00f, 0.65f, 0.40f, 0.60f } },
    { 'm', { 0.20f, 0.10f, 0.95f, 0.12f, 0.60f } },
    { 'n', { 0.88f, 0.50f, 0.83f, 0.10f, 0.74f } },
    { 'p', { 0.00f, 0.00f, 0.81f, 0.40f, 0.60f } },
    { 'b', { 0.00f, 0.00f, 0.76f, 0.57f, 0.60f } },
    { 'r', { 0.00f, 0.20f, 0.55f, 0.68f, 0.60f } },
    { 'v', { 0.95f, 0.20f, 0.97f, 0.67f, 0.70f } },
    { 'f', { 0.95f, 0.20f, 0.97f, 0.75f, 0.00f } },
    { 'd', { 0.89f, 0.37f, 0.91f, 0.60f, 0.74f } },
    { 't', { 0.95f, 0.88f, 0.94f, 0.58f, 0.00f } },
    { 'c', { 0.50f, 0.21f, 0.74f, 0.58f, 0.56f } },
    { 'h', { 0.00f, 1.00f, 0.10f, 0.59f, 0.02f } },
    { 'j', { 0.26f, 0.48f, 0.71f, 0.61f, 0.91f } },
    { 'w', { 0.00f, 0.50f, 0.95f, 0.68f, 0.41f } },
    { 'x', { 0.53f, 0.47f, 0.13f, 0.09f, 0.60f } }
};

static const model::VoiceProcessor::ControlPoint& getControlPointForPhoneme(char sym)
{
    const static model::VoiceProcessor::ControlPoint dummy{};

    auto it{ phonemes.find(sym) };

    if (it != phonemes.end())
        return it->second;

    return dummy;
}

static float getNoteFrequency(int noteNumber)
{
    return 440.0f * powf(2.0f, float(noteNumber - 69) / 12.0f);
}

template<size_t S>
static size_t phonemesFromString(const String& str, std::array<Voice::Phoneme, S>& target)
{
    const char* aStr{ str.getCharPointer().getAddress() };
    size_t i = 0;

    while (i < target.size() && aStr[i] != '\0') {
        target[i].symbol = aStr[i];
        ++i;
    }

    return i;
}

//==============================================================================

void Voice::Phrase::parse(const String& a, const String& r)
{
    const char* aStr{ a.getCharPointer().getAddress() };
    const char* rStr{ r.getCharPointer().getAddress() };

    numAttackPhonemes = phonemesFromString(aStr, attack);
    numReleasePhonemes = phonemesFromString(rStr, release);
}

//==============================================================================

Voice::Voice(Engine& eng)
    : engine{ eng }
{
}

void Voice::prepareToPlay(float sampleRate, int samplesPerBlock)
{
    voiceProcessor.prepareToPlay(sampleRate, samplesPerBlock);
}

void Voice::trigger(const Trigger& t)
{
    triggerRecord = t;

    jassert(triggerRecord.phrase.numAttackPhonemes > 0);

    attackPhase = true;
    phonemeIndex = 0;
    generatedSamplesInPhoneme = 0;
    totalSamplesInPhoneme = Engine::INTERNAL_SAMPLE_RATE * triggerRecord.phrase.attack[0].duration;

    voiceProcessor.setFrequency(getNoteFrequency(triggerRecord.key), true);
    voiceProcessor.setTenseness(triggerRecord.phrase.tenseness);

    const auto cp{ getControlPointForPhoneme(triggerRecord.phrase.attack[0].symbol) };
    voiceProcessor.trigger(cp);

    envelope.trigger(triggerRecord.envelope);
}

void Voice::retrigger(const Trigger& t)
{
    triggerRecord = t;

    jassert(triggerRecord.phrase.numAttackPhonemes > 0);

    attackPhase = true;
    phonemeIndex = 0;
    generatedSamplesInPhoneme = 0;
    totalSamplesInPhoneme = Engine::INTERNAL_SAMPLE_RATE * triggerRecord.phrase.attack[0].duration;

    voiceProcessor.setFrequency(getNoteFrequency(triggerRecord.key), false);
    voiceProcessor.setTenseness(triggerRecord.phrase.tenseness);

    const auto cp{ getControlPointForPhoneme(triggerRecord.phrase.attack[0].symbol) };
    voiceProcessor.retrigger(cp);
    voiceProcessor.setVibrato(0.0f);

    envelope.retrigger();
}

void Voice::release()
{
    if (triggerRecord.phrase.numReleasePhonemes > 0) {
        attackPhase = false;
        phonemeIndex = 0;
        generatedSamplesInPhoneme = 0;

        const auto cp{ getControlPointForPhoneme(triggerRecord.phrase.release[0].symbol) };
        voiceProcessor.setControlPoint(cp);
        voiceProcessor.setVibrato(0.0f);
    } else {
        // There is no release portion in the phrase
        envelope.release();
        voiceProcessor.release();
    }
}

void Voice::process(float* outL, float* outR, size_t numFrames)
{
    voiceProcessor.process(outL, (int)numFrames);

    // Apply envelope
    for (size_t i = 0; i < numFrames; ++i) {
        outL[i] *= envelope.getNext();
    }

    if (outR != outL)
        memcpy(outR, outL, sizeof(float) * numFrames);

    generatedSamplesInPhoneme += numFrames;

    if (generatedSamplesInPhoneme >= totalSamplesInPhoneme) {
        // @note This will consume some time from the next phoneme to make transisions block-aligned
        generatedSamplesInPhoneme -= totalSamplesInPhoneme;

        if (attackPhase) {
            if (phonemeIndex < triggerRecord.phrase.numAttackPhonemes - 1) {
                ++phonemeIndex;
                const char nextSymbol{ triggerRecord.phrase.attack[phonemeIndex].symbol };
                const auto cp{ getControlPointForPhoneme(nextSymbol) };
                voiceProcessor.setControlPoint(cp);
                totalSamplesInPhoneme = Engine::INTERNAL_SAMPLE_RATE * triggerRecord.phrase.attack[phonemeIndex].duration;
            } else {
                // Sustain the last phoneme
                if (vibratoLevel < 1.0f) {
                    vibratoLevel += 0.01f + vibratoLevel * 0.02f;
                    vibratoLevel = jlimit(0.0f, 1.0f, vibratoLevel);
                }

                voiceProcessor.setVibrato(vibratoLevel);
            }
        } else {
            if (phonemeIndex < triggerRecord.phrase.numReleasePhonemes - 1) {
                ++phonemeIndex;
                const char nextSymbol{ triggerRecord.phrase.release[phonemeIndex].symbol };
                const auto cp{ getControlPointForPhoneme(nextSymbol) };
                voiceProcessor.setControlPoint(cp);
                totalSamplesInPhoneme = Engine::INTERNAL_SAMPLE_RATE * triggerRecord.phrase.release[phonemeIndex].duration;
            } else {
                // Release on the last phoneme
                envelope.release();
                voiceProcessor.release();
                generatedSamplesInPhoneme = 0;
            }
        }
    }
}

bool Voice::isReleasing() const
{
    return envelope.getState() == Envelope::State::Release;
}

bool Voice::isOver() const
{
    return envelope.getState() == Envelope::State::Off;
}

void Voice::reset()
{
    vibratoLevel = 0.0f;
}

//==============================================================================

VoicePool::VoicePool(Engine& eng, size_t numVoices)
    : engine{ eng },
      voices(numVoices, engine),
      idleVoices{},
      voiceCount{ 0 }
{
    for (auto& voice : voices)
        idleVoices.append(&voice);
}

void VoicePool::prepareToPlay(float sampleRate, int samplesPerBlock)
{
    for (auto& voice : voices)
        voice.prepareToPlay(sampleRate, samplesPerBlock);
}

Voice* VoicePool::trigger(const Voice::Trigger& trigger)
{
    if (auto* voice{ idleVoices.first() }) {
        idleVoices.remove(voice);
        voice->trigger(trigger);
        ++voiceCount;
        return voice;
    }

    return nullptr;
}

void VoicePool::recycle(Voice* voice)
{
    jassert(voice != nullptr);
    voice->reset();
    idleVoices.append(voice);
    --voiceCount;
    jassert(voiceCount >= 0);
}

} // namespace engine
