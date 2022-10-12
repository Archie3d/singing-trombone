#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace juce;

SingingTromboneEditor::SingingTromboneEditor (SingingTromboneProcessor& p)
    : AudioProcessorEditor(&p),
      audioProcessor(p),
      lyrics(),
      lyricsEditor(lyrics, nullptr),
      updateButton("Update")
{
    setSize (400, 300);
    setResizeLimits(200, 200, 4096, 4096);

    addAndMakeVisible(lyricsEditor);
    addAndMakeVisible(updateButton);

    updateButton.onClick = [this]() {
        updateLyrics();
    };

    resized();
    startTimerHz(10);
}

SingingTromboneEditor::~SingingTromboneEditor() = default;

void SingingTromboneEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void SingingTromboneEditor::resized()
{
    auto bounds{ getLocalBounds() };
    auto bottom{ bounds.removeFromBottom(40) };
    bottom.reduce(6, 6);
    updateButton.setBounds(bottom.removeFromRight(80));

    lyricsEditor.setBounds(bounds);
}

void SingingTromboneEditor::updateLyrics()
{
    auto res{ audioProcessor.setLyrics(lyrics.getAllContent()) };

    if (res.failed()) {
        AlertWindow::showMessageBoxAsync(MessageBoxIconType::WarningIcon, "Error", res.getErrorMessage(), "OK");
    }
}

void SingingTromboneEditor::timerCallback()
{
    // @todo Update meters 
}
