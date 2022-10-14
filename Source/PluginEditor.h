#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class SingingTromboneEditor : public juce::AudioProcessorEditor,
                              public juce::Timer,
                              public SingingTromboneProcessor::Listener,
                              public Slider::Listener,
                              public Button::Listener
{
public:
    SingingTromboneEditor (SingingTromboneProcessor&);
    ~SingingTromboneEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void updateLyrics();

    // juce::Timer
    void timerCallback() override;

    // SingingTromboneProcessor::Listener
    void processorStateChanged() override;

    // Slider::Listener
    void sliderValueChanged(Slider* slider) override;

    // Button::Listener
    void buttonClicked(Button* b) override;

private:

    SingingTromboneProcessor& audioProcessor;

    CodeEditorComponent lyricsEditor;
    TextButton updateButton;

    Slider volumeSlider{ Slider::LinearVertical, Slider::NoTextBox };
    Slider expressionSlider{ Slider::LinearVertical, Slider::NoTextBox };

    Slider attackSlider{ Slider::Rotary, Slider::NoTextBox };
    Slider decaySlider{ Slider::Rotary, Slider::NoTextBox };
    Slider sustainSlider{ Slider::Rotary, Slider::NoTextBox };
    Slider releaseSlider{ Slider::Rotary, Slider::NoTextBox };

    Slider vibratoSlider{ Slider::LinearVertical, Slider::NoTextBox };

    ToggleButton legatoToggleButton{ "Legato" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SingingTromboneEditor)
};
