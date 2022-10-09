// ----------------------------------------------------------------------------
//
//  Copyright (C) 2022 Arthur Benilov <arthur.benilov@gmail.com>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// ----------------------------------------------------------------------------

#include <chrono>

#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace juce;

SingingTromboneProcessor::SingingTromboneProcessor()
    : AudioProcessor(getBusesProperties())
{
}

SingingTromboneProcessor::~SingingTromboneProcessor() = default;

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
    
    const auto totalNumInputChannels { getTotalNumInputChannels() };
    const auto totalNumOutputChannels{ getTotalNumOutputChannels() };

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    processMidi (midiMessages);


    float* outL = buffer.getWritePointer(0);
    float* outR = outL;

    if (totalNumOutputChannels > 1)
        outR = buffer.getWritePointer(1);

    // @todo
    buffer.clear();

    auto timestampStop{ high_resolution_clock::now() };
    auto duration_us{ duration_cast<microseconds> (timestampStop - timestampStart).count() };

    const float realTime_us{ 1e6f * (float) buffer.getNumSamples() / 44100.0f }; // @todo get sample rate from prepareToPlay
    const float load{ duration_us / realTime_us };

    processLoad = jmin(1.0f, 0.99f * processLoad + 0.01f * load);
}

void SingingTromboneProcessor::processMidi(MidiBuffer& midiMessages)
{
    if (midiMessages.getNumEvents() == 0)
        return;

    for (auto msgIter : midiMessages) {
        const auto msg = msgIter.getMessage();

        // Handle CCs
        const int ch = msg.getChannel();

        if (msg.isController()) {
            const int cc{ msg.getControllerNumber() };
            const float value{ float(msg.getControllerValue()) / 127.0f };

            switch (cc) {
            default:
                break;
            }
        }

        // @todo process notes on/off
        // processMIDIMessage(msg);
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
}

void SingingTromboneProcessor::setStateInformation (const void* data, int sizeInBytes)
{
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
