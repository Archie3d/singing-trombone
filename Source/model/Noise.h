#pragma once

#include <JuceHeader.h>

namespace model {

class WhiteNoise
{
public:
    WhiteNoise();
    void setSeed(int64 seed);
    float tick();
private:
    juce::Random random{};
};

} // namespace model
