#include "LookAndFeel.h"

CustomLookAndFeel::CustomLookAndFeel()
{
    setColour(ResizableWindow::backgroundColourId, Colour(0x2F, 0x2F, 0x2F));

    setColour(Slider::thumbColourId,               Colour(0xCC, 0xCC, 0xCC));
    setColour(Slider::textBoxOutlineColourId,      Colours::transparentWhite);
    setColour(Slider::trackColourId,               Colour(0x10, 0x66, 0x99));
    setColour(Slider::rotarySliderFillColourId,    Colour(0x10, 0x66, 0x99));
    setColour(Slider::rotarySliderOutlineColourId, Colour(0x1F, 0x1F, 0x1F));
    setColour(Slider::backgroundColourId,          Colour(0x1F, 0x1F, 0x1F));

    setColour(TextButton::buttonColourId,   Colour(0x10, 0x99, 0x66));
    setColour(TextButton::textColourOffId,  Colours::lightgrey);
    setColour(TextButton::buttonOnColourId, Colours::lightgrey);
    setColour(TextButton::textColourOnId,   Colours::white);

    setColour(CodeEditorComponent::backgroundColourId,     Colour(0x10, 0x10, 0x10));
    setColour(CodeEditorComponent::defaultTextColourId,    Colours::lightyellow);
    setColour(CodeEditorComponent::highlightColourId,      Colours::darkmagenta);
    setColour(CodeEditorComponent::lineNumberTextId,       Colour(0x10, 0x99, 0x66));
    setColour(CodeEditorComponent::lineNumberBackgroundId, Colour(0x1F, 0x1F, 0x1F));
}

CustomLookAndFeel& CustomLookAndFeel::getInstance()
{
    static CustomLookAndFeel instance;
    return instance;
}
