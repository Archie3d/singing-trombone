#pragma once

#include <JuceHeader.h>

namespace model {

class Tract
{
public:
    struct Config
    {
        constexpr static int defaultNumSegments = 44;

        int n{ defaultNumSegments };
        int lipStart{ 39 };
        int bladeStart{ 10 };
        int tipStart{ 32 };
        int noseLength{ 28 };
        int noseStart{ n - noseLength + 1 };
        float noseOffset{ 0.8f };
        float tongueIndex{ (float)bladeStart };
        float tongueDiameter{ noseLength };

        std::vector<float> noseDiameter{};
        std::vector<float> tractDiameter{};

        Config(int numSegments = defaultNumSegments);
    };
};

} // namespace model
