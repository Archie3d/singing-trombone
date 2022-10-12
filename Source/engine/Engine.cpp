#include "engine/Engine.h"

namespace engine {

Engine::Engine()
    : voicePool(*this)
{
}

void Engine::prepareToPlay(float sr, int samplesPerBlock)
{
    sampleRate = sr;
    voicePool.prepareToPlay(sampleRate, samplesPerBlock);
    mixBuffer.setSize(2, samplesPerBlock);
    keysState.reset();
    sustained = false;
}

void Engine::process(float* outL, float* outR, size_t numFrames)
{
    jassert(numFrames <= mixBuffer.getNumSamples());

    auto* voice{ activeVoices.first() };

    while (voice != nullptr) {
        float* mixL{ mixBuffer.getWritePointer(0) };
        float* mixR{ mixBuffer.getWritePointer(1) };
        voice->process(mixL, mixR, numFrames);

        for (size_t i = 0; i < numFrames; ++i) {
            outL[i] += mixL[i];
            outR[i] += mixR[i];
        }

        if (voice->isOver()) {
            auto* nextVoice{ activeVoices.removeAndReturnNext(voice) };
            voicePool.recycle(voice);
            voice = nextVoice;
        } else {
            voice = voice->next();
        }
    }

    updateParameters(numFrames);
}

void Engine::processMidiMessage(const MidiMessage& msg)
{
    if (msg.isNoteOn())
        noteOn(msg);
    else if (msg.isNoteOff())
        noteOff(msg);
    else if (msg.isController())
        controlChange(msg);
}

void Engine::processLyrics()
{
    Lyrics::Ptr lyricsPtr{};
    Lyrics::Ptr ptr{};

    do {
        if (lyricsPtr != nullptr)
            disposeLyricsQueue.send(lyricsPtr);

        lyricsPtr = ptr;
    } while (setLyricsQueue.receive(ptr));

    if (lyricsPtr == nullptr)
        return;

    jassert(lyricsPtr == ptr);

    size_t numPhrases{ jmin(lyricsPtr->size(), lyricsMaxPhrases) };

    for (size_t i = 0; i < numPhrases; ++i) {
        const auto& phrase{ lyricsPtr->operator[](i) };

        lyrics[i].parse(phrase.attack, phrase.release);
    }

    lyricsNumPhrases = numPhrases;
    phraseIndex = 0;

    disposeLyricsQueue.send(lyricsPtr);
}

Result Engine::setLyrics(const String& str)
{
    auto lyricsPtr{ std::make_shared<Lyrics>() };
    auto res{ lyricsPtr->parse(str) };

    if (res.wasOk()) {
        return setLyrics(lyricsPtr);
    }

    return res;
}

void Engine::rewind()
{
    phraseIndex = 0;
}

Result Engine::setLyrics(const Lyrics::Ptr& ptr)
{
    if (setLyricsQueue.send(ptr)) {
        cachedLyrics = ptr;
        return Result::ok();
    }

    return Result::fail("Queue is full");
}

Lyrics::Phrase Engine::getCurrentPhrase() const
{
    if (cachedLyrics != nullptr) {
        size_t idx{ getCurrentPhraseIndex() };

        if (idx < cachedLyrics->size()) {
            return cachedLyrics->operator[](idx);
        }
    }

    return {};
}

void Engine::performHousekeeping()
{
    Lyrics::Ptr ptr{};

    while (disposeLyricsQueue.receive(ptr))
        ptr.reset();
}

void Engine::updateParameters(size_t numFrames)
{
    // @todo
}

void Engine::noteOn(const MidiMessage& msg)
{
    keysState.set(msg.getNoteNumber());

    if (phraseIndex >= lyricsNumPhrases) {
        // No lyrics available
        return;
    }

    Voice::Trigger trigger{};
    trigger.key = msg.getNoteNumber();
    trigger.velocity = (float)msg.getVelocity() / 127.0f;

    trigger.phrase = lyrics[phraseIndex];
    phraseIndex = (phraseIndex + 1) % lyricsNumPhrases;

    // @todo Populate tenseness from global parameters
    trigger.phrase.tenseness = 0.6f;

    bool triggered{ false };

    if (legato) {
        if (auto* voice{ activeVoices.first() }) {
            if (!voice->isReleasing()) {
                voice->retrigger(trigger);
                triggered = true;
            }
        }
    }

    if (!triggered) {
        if (auto* voice{ voicePool.trigger(trigger) }) {
            activeVoices.append(voice);
        }
    }
}

void Engine::noteOff(const MidiMessage& msg)
{
    keysState.reset(msg.getNoteNumber());

    if (sustained)
        return;

    auto* voice{ activeVoices.first() };

    while (voice != nullptr) {
        if (voice->getTriggerRecord().key == msg.getNoteNumber())
            voice->release();

        voice = voice->next();
    }
}

void Engine::controlChange(const MidiMessage& msg)
{
    constexpr int CC_SUSTAIN = 64;

    if (msg.getControllerNumber() == CC_SUSTAIN) {
        bool wasSustained{ sustained };
        sustained = msg.getControllerValue() > 63;

        if (wasSustained && (!sustained)) {
            releaseSustainedVoices();
        }
    }
}

void Engine::releaseSustainedVoices()
{
    auto* voice{ activeVoices.first() };

    while (voice != nullptr) {
        if (!keysState[voice->getTriggerRecord().key])
            voice->release();

        voice = voice->next();
    }
}

} // namespace engine
