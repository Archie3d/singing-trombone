#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace juce;

SingingTromboneEditor::SingingTromboneEditor (SingingTromboneProcessor& p)
    : AudioProcessorEditor(&p)
    , audioProcessor(p)
{
    setSize (400, 300);
    setResizeLimits(200, 200, 4096, 4096);

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
}

void SingingTromboneEditor::timerCallback()
{
    // @todo Update meters 
}
