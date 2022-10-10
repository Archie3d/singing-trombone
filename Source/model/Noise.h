#pragma once

#include <JuceHeader.h>
#include <array>

namespace model {

class WhiteNoise final
{
public:
    WhiteNoise();
    void setSeed(int64 seed);
    float tick();
private:
    juce::Random random{};
};

//==============================================================================

class SimplexNoise final
{
public:
    SimplexNoise();
    void setSeed(int64 seed);

    float sample2d(float x, float y);
    float sample1d(float x);

private:
    struct Grad
    {
        float x{};
        float y{};

        float dot(float a, float b) const { return x*a + y*b; }
    };

    std::array<int, 512> perm{};
    std::array<Grad, 512> gradP{};
};

} // namespace model
