#include <chrono>

#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace juce;

SingingTromboneProcessor::SingingTromboneProcessor()
    : AudioProcessor(getBusesProperties()),
      parameters(*this)
{
    startTimerHz(30);
}

SingingTromboneProcessor::~SingingTromboneProcessor() = default;

void SingingTromboneProcessor::addListener(Listener* listener)
{
    jassert(listener != nullptr);
    listeners.add(listener);
}

void SingingTromboneProcessor::removeListener(Listener* listener)
{
    listeners.remove(listener);
}

const juce::String SingingTromboneProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SingingTromboneProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool SingingTromboneProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool SingingTromboneProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double SingingTromboneProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SingingTromboneProcessor::getNumPrograms()
{
    // @note Should be at least 1
    return 1;
}

int SingingTromboneProcessor::getCurrentProgram()
{
    return 1;
}

void SingingTromboneProcessor::setCurrentProgram([[maybe_unused]] int index)
{
}

const String SingingTromboneProcessor::getProgramName(int index)
{
    return "Default";
}

void SingingTromboneProcessor::changeProgramName(int /* index */, const String& /* newName */)
{
}

void SingingTromboneProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    engine.prepareToPlay((float)sampleRate, samplesPerBlock);
}

void SingingTromboneProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations

bool SingingTromboneProcessor::canAddBus([[maybe_unused]] bool isInput) const
{
    return false;
}

bool SingingTromboneProcessor::canRemoveBus([[maybe_unused]] bool isInput) const
{
    return false;
}

bool SingingTromboneProcessor::canApplyBusCountChange([[maybe_unused]] bool isInput,
                                                  [[maybe_unused]] bool isAdding,
                                                  [[maybe_unused]] BusProperties& outProperties)
{
    return false;
}

bool SingingTromboneProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    static_assert(!JucePlugin_IsMidiEffect, "This plugin is not a MIDI effect");
    static_assert(JucePlugin_IsSynth, "This plugin is a synthesizer");

    // No inputs are expected.
    if (layouts.inputBuses.size() > 0)
        return false;

    if (layouts.outputBuses.size() == 1 && layouts.outputBuses.getUnchecked(0) == AudioChannelSet::stereo())
        return true;

    return false;
}

void SingingTromboneProcessor::processorLayoutsChanged()
{
}

#endif // JucePlugin_PreferredChannelConfigurations

void SingingTromboneProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    using namespace std::chrono;

    auto timestampStart{ high_resolution_clock::now() };

    juce::ScopedNoDenormals noDenormals;

    if (auto* playHead{ getPlayHead() }) {
        AudioPlayHead::CurrentPositionInfo pos{};
        playHead->getCurrentPosition(pos);

        if (pos.timeInSamples < timeInSamples) {
            engine.rewind();
        }

        timeInSamples = pos.timeInSamples;
    }

    updateParameters();

    const auto totalNumInputChannels { getTotalNumInputChannels() };
    const auto totalNumOutputChannels{ getTotalNumOutputChannels() };

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    processMidi(midiMessages);

    engine.processLyrics();

    float* outL = buffer.getWritePointer(0);
    float* outR = outL;

    if (totalNumOutputChannels > 1)
        outR = buffer.getWritePointer(1);

    buffer.clear();

    engine.process(outL, outR, (size_t)buffer.getNumSamples());

    auto timestampStop{ high_resolution_clock::now() };
    auto duration_us{ duration_cast<microseconds>(timestampStop - timestampStart).count() };

    const float realTime_us{ 1e6f * (float)buffer.getNumSamples() / engine.getExternalSampleRate() }; // @todo get sample rate from prepareToPlay
    const float load{ duration_us / realTime_us };

    processLoad = jmin(1.0f, 0.99f * processLoad + 0.01f * load);
}

void SingingTromboneProcessor::processMidi(MidiBuffer& midiMessages)
{
    if (midiMessages.getNumEvents() == 0)
        return;

    for (auto msgIter : midiMessages) {
        const auto msg{ msgIter.getMessage() };

        if (msg.isController()) {
            const float value{ float(msg.getControllerValue()) / 127.0f };

            switch (msg.getControllerNumber()) {
                case 1: // Modulation
                    parameters.vibratoIntensity->setValueNotifyingHost(value);
                    break;
                case 7: // Volume
                    parameters.volume->setValueNotifyingHost(value);
                    break;
                case 11:    // Expression
                    parameters.expression->setValueNotifyingHost(value);
                    break;
                case 72:    // Release
                    parameters.envelopeRelease->setValueNotifyingHost(value);
                    break;
                case 73:    // Attack
                    parameters.envelopeAttack->setValueNotifyingHost(value);
                    break;
                case 80:    // Decay
                    parameters.envelopeDecay->setValueNotifyingHost(value);
                    break;
                case 126:   // Mono mode
                    parameters.legatoEnabled->setValueNotifyingHost(true);
                    break;
                case 127:   // Poly mode
                    parameters.legatoEnabled->setValueNotifyingHost(false);
                    break;
                default:
                    break;
            }
        }

        engine.processMidiMessage(msg);
    }
}

bool SingingTromboneProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* SingingTromboneProcessor::createEditor()
{
    return new SingingTromboneEditor(*this);
}

void SingingTromboneProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    parameters.lyrics = lyricsDocument.getAllContent();

    MemoryOutputStream stream(destData, false);
    parameters.serialize(stream);
}

void SingingTromboneProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    jassert(data != nullptr);

    MemoryInputStream stream(data, sizeInBytes, false);
    parameters.deserialize(stream);

    lyricsDocument.replaceAllContent(parameters.lyrics);
    updateLyrics();
    updateParameters();

    listeners.call(&Listener::processorStateChanged);
}

Result SingingTromboneProcessor::updateLyrics()
{
    return engine.setLyrics(lyricsDocument.getAllContent());
}

void SingingTromboneProcessor::updateParameters()
{
    engine.setVolume(parameters.volume->get());
    engine.setExpression(parameters.expression->get());
    engine.setEnvelopeAttack(parameters.envelopeAttack->get());
    engine.setEnvelopeDecay(parameters.envelopeDecay->get());
    engine.setEnvelopeSustain(parameters.envelopeSustain->get());
    engine.setEnvelopeRelease(parameters.envelopeRelease->get());

    engine.setLegato(parameters.legatoEnabled->get());
    engine.setVibrato(parameters.vibratoIntensity->get());
}

//==============================================================================

void SingingTromboneProcessor::timerCallback()
{
    engine.performHousekeeping();
}

//==============================================================================

AudioProcessor::BusesProperties SingingTromboneProcessor::getBusesProperties()
{
    BusesProperties buses{};
    buses = buses.withOutput("Output", AudioChannelSet::stereo(), true);
    return buses;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SingingTromboneProcessor();
}
