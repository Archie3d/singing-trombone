#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class SingingTromboneEditor : public juce::AudioProcessorEditor,
                              public juce::Timer,
                              public SingingTromboneProcessor::Listener,
                              public Slider::Listener,
                              public Button::Listener,
                              public CodeDocument::Listener
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

    // CodeDocument::Listener
    void codeDocumentTextInserted(const String& newText, int insertIndex) override;
    void codeDocumentTextDeleted(int startIndex, int endIndex) override;

private:

    void onLyricsChanged();

    SingingTromboneProcessor& audioProcessor;

    Label loadLabel{ {}, { "CPU load:"} };
    Label loadValueLabel{};
    Label voicesLabel{ {}, { "Voices:"} };
    Label voicesValueLabel{};

    CodeEditorComponent lyricsEditor;
    TextButton updateButton{ "Update lyrics "};

    Slider volumeSlider{ Slider::LinearVertical, Slider::NoTextBox };
    Slider expressionSlider{ Slider::LinearVertical, Slider::NoTextBox };

    Slider attackSlider{ Slider::Rotary, Slider::NoTextBox };
    Slider decaySlider{ Slider::Rotary, Slider::NoTextBox };
    Slider sustainSlider{ Slider::Rotary, Slider::NoTextBox };
    Slider releaseSlider{ Slider::Rotary, Slider::NoTextBox };

    Slider vibratoSlider{ Slider::LinearVertical, Slider::NoTextBox };

    ToggleButton legatoToggleButton{ "Legato" };

    bool lyricsChanged{};
    Range<int> highlightRegion{};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SingingTromboneEditor)
};
