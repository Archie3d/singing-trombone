#pragma once

#include <JuceHeader.h>
#include <atomic>

#include "PluginParameters.h"
#include "engine/Engine.h"

class SingingTromboneProcessor : public juce::AudioProcessor,
                                 private juce::Timer
{
public:

    class Listener
    {
    public:
        virtual void processorStateChanged() = 0;
        virtual ~Listener() = default;
    };

    SingingTromboneProcessor();
    ~SingingTromboneProcessor();

    void addListener(Listener* listener);
    void removeListener(Listener* listener);

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool canAddBus(bool isInput) const override;
    bool canRemoveBus(bool isInput) const override;
    bool canApplyBusCountChange(bool isInput, bool isAdding, BusProperties& outProperties) override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processorLayoutsChanged() override;
#endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void processMidi(juce::MidiBuffer& midiMessages);

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    float getProcessLoad() const noexcept { return processLoad.load(); }
    int getActiveVoiceCount() const noexcept { return engine.getVoiceCount(); }

    CodeDocument& getLyricsDocument() { return lyricsDocument; }

    Result updateLyrics();

    void updateParameters();

    PluginParameters& getParametersContainer() { return parameters; }

private:

    // juce::Timer
    void timerCallback() override;

    static BusesProperties getBusesProperties();

    engine::Engine engine{};
    std::atomic<float> processLoad{};

    int64 timeInSamples{};

    ListenerList<Listener> listeners{};

    CodeDocument lyricsDocument{};

    PluginParameters parameters;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SingingTromboneProcessor)
};
