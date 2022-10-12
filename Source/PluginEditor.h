#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class SingingTromboneEditor : public juce::AudioProcessorEditor,
                              public juce::Timer
{
public:
    SingingTromboneEditor (SingingTromboneProcessor&);
    ~SingingTromboneEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void updateLyrics();

    // juce::Timer
    void timerCallback() override;

private:

    SingingTromboneProcessor& audioProcessor;

    CodeDocument lyrics;
    CodeEditorComponent lyricsEditor;
    TextButton updateButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SingingTromboneEditor)
};
