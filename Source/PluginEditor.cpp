#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace juce;

SingingTromboneEditor::SingingTromboneEditor (SingingTromboneProcessor& p)
    : AudioProcessorEditor(&p),
      audioProcessor(p),
      lyricsEditor(audioProcessor.getLyricsDocument(), nullptr),
      updateButton("Update")
{
    setSize(600, 400);
    setResizeLimits(200, 200, 4096, 4096);

    lyricsEditor.setScrollbarThickness(8);
    addAndMakeVisible(lyricsEditor);
    addAndMakeVisible(updateButton);
    
    auto& params{ audioProcessor.getParametersContainer() };

    volumeSlider.setRange(params.volume->range.start, params.volume->range.end);
    expressionSlider.setRange(params.expression->range.start, params.expression->range.end);

    attackSlider.setRange(params.envelopeAttack->range.start, params.envelopeAttack->range.end);
    decaySlider.setRange(params.envelopeDecay->range.start, params.envelopeDecay->range.end);
    sustainSlider.setRange(params.envelopeSustain->range.start, params.envelopeSustain->range.end);
    releaseSlider.setRange(params.envelopeRelease->range.start, params.envelopeRelease->range.end);

    vibratoSlider.setRange(params.vibratoIntensity->range.start, params.vibratoIntensity->range.end);

    volumeSlider.addListener(this);
    expressionSlider.addListener(this);
    attackSlider.addListener(this);
    decaySlider.addListener(this);
    sustainSlider.addListener(this);
    releaseSlider.addListener(this);
    vibratoSlider.addListener(this);
    legatoToggleButton.addListener(this);

    addAndMakeVisible(volumeSlider);
    addAndMakeVisible(expressionSlider);
    addAndMakeVisible(attackSlider);
    addAndMakeVisible(decaySlider);
    addAndMakeVisible(sustainSlider);
    addAndMakeVisible(releaseSlider);
    addAndMakeVisible(vibratoSlider);
    addAndMakeVisible(legatoToggleButton);

    updateButton.onClick = [this]() {
        updateLyrics();
    };

    resized();

    audioProcessor.addListener(this);

    startTimerHz(10);
}

SingingTromboneEditor::~SingingTromboneEditor()
{
    audioProcessor.removeListener(this);
}

void SingingTromboneEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void SingingTromboneEditor::resized()
{
    auto bounds{ getLocalBounds() };
    auto bottom{ bounds.removeFromBottom(80) };
    bottom.reduce(6, 6);

    volumeSlider.setBounds(bottom.removeFromLeft(40));
    expressionSlider.setBounds(bottom.removeFromLeft(40));

    attackSlider.setBounds(bottom.removeFromLeft(75));
    decaySlider.setBounds(bottom.removeFromLeft(75));
    sustainSlider.setBounds(bottom.removeFromLeft(75));
    releaseSlider.setBounds(bottom.removeFromLeft(75));

    vibratoSlider.setBounds(bottom.removeFromLeft(40));
    legatoToggleButton.setBounds(bottom.removeFromLeft(100));

    updateButton.setBounds(bottom.removeFromRight(80));

    lyricsEditor.setBounds(bounds);
}

void SingingTromboneEditor::updateLyrics()
{
    auto res{ audioProcessor.updateLyrics() };

    if (res.failed()) {
        AlertWindow::showMessageBoxAsync(MessageBoxIconType::WarningIcon, "Error", res.getErrorMessage(), "OK");
    }
}

void SingingTromboneEditor::timerCallback()
{
    auto& params{ audioProcessor.getParametersContainer() };
    volumeSlider.setValue(params.volume->get(), dontSendNotification);
    expressionSlider.setValue(params.expression->get(), dontSendNotification);
    attackSlider.setValue(params.envelopeAttack->get(), dontSendNotification);
    decaySlider.setValue(params.envelopeDecay->get(), dontSendNotification);
    sustainSlider.setValue(params.envelopeSustain->get(), dontSendNotification);
    releaseSlider.setValue(params.envelopeRelease->get(), dontSendNotification);
    vibratoSlider.setValue(params.vibratoIntensity->get(), dontSendNotification);

    legatoToggleButton.setToggleState(params.legatoEnabled->get(), dontSendNotification);
}

void SingingTromboneEditor::processorStateChanged()
{
    //
}

void SingingTromboneEditor::sliderValueChanged(Slider* slider)
{
    auto& params{ audioProcessor.getParametersContainer() };
    const float value{ (float)slider->getValue() };

    if (slider == &volumeSlider)
        params.volume->setValueNotifyingHost(params.volume->convertTo0to1(value));
    else if (slider == &expressionSlider)
        params.expression->setValueNotifyingHost(params.expression->convertTo0to1(value));
    else if (slider == &attackSlider)
        params.envelopeAttack->setValueNotifyingHost(params.envelopeAttack->convertTo0to1(value));
    else if (slider == &decaySlider)
        params.envelopeDecay->setValueNotifyingHost(params.envelopeDecay->convertTo0to1(value));
    else if (slider == &sustainSlider)
        params.envelopeSustain->setValueNotifyingHost(params.envelopeSustain->convertTo0to1(value));
    else if (slider == &releaseSlider)
        params.envelopeRelease->setValueNotifyingHost(params.envelopeRelease->convertTo0to1(value));
    else if (slider == &vibratoSlider)
        params.vibratoIntensity->setValueNotifyingHost(params.vibratoIntensity->convertTo0to1(value));
}

void SingingTromboneEditor::buttonClicked(Button* b)
{
    auto& params{ audioProcessor.getParametersContainer() };

    if (b == dynamic_cast<Button*>(&legatoToggleButton)) {
        params.legatoEnabled->setValueNotifyingHost(legatoToggleButton.getToggleState());
    }
}
