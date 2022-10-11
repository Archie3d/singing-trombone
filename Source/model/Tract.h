#pragma once

#include <JuceHeader.h>
#include <vector>

#include "model/Glottis.h"

namespace model {

class Tract final
{
public:
    struct Config final
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
        float tongueDiameter{ (float)noseLength };

        std::vector<float> noseDiameter{};
        std::vector<float> tractDiameter{};

        Config(int numSegments = defaultNumSegments);
    };

    struct Transient final
    {
        int position{};
        float timeAlive{};
        float lifeTime{};
        float strength{};
        float exponent{};
        bool living{};
    };

    Tract();

    void reset();
    void prepareToPlay(float sampleRate, float blockTime);
    void tick(float glottalOutput, float turbulenceNoise, float lambda, Glottis& glottis);
    void finishBlock();

    void setRestDiameter(float tongueIndex, float tongueDiameter);
    void setConstriction(float cindex, float cdiam, float fricativeIntensity);

    int getTractIndexCount() const;
    int getTongueIndexLowerBound() const;
    int getTongueIndexUpperBound() const;

    float getLipOutput() const noexcept { return lipOutput; }
    float getNoseOutput() const noexcept { return noseOutput; }

private:

    void initialize();
    void calculateReflections();
    void calculateNoseReflections();
    void addTransient(int position);
    void addTurbulenceNoise(float turbulenceNoise, Glottis& glottis);
    void addTurbulenceNoiseAtIndex(float turbulenceNoise, float index, float d, Glottis& glottis);
    void reshapeTract(float deltaTime);
    void processTransients();

    Config config{};
    float sampleRate{ 44100.0f };
    float blockTime{ 512.0f / sampleRate };

    float glottalReflection{ 0.75f };
    float lipReflection{ -0.85f };
    int lastObstruction{ -1 };
    float fade{ 1.0f };
    float movementSpeed{ 15.0f };
    float velumTarget{ 0.01f };

    constexpr static size_t maxTransients = 20;
    std::array<Transient, maxTransients> transients;
    size_t transientCount{};

    std::vector<float> diameter;
    std::vector<float> restDiameter;
    std::vector<float> targetDiameter;
    std::vector<float> newDiameter;

    std::vector<float> L;
    std::vector<float> R;
    std::vector<float> reflection;
    std::vector<float> newReflection;
    std::vector<float> junctionOutputL;
    std::vector<float> junctionOutputR;
    std::vector<float> A;
    std::vector<float> maxAmplitude;

    std::vector<float> noseL;
    std::vector<float> noseR;
    std::vector<float> noseJunctionOutputL;
    std::vector<float> noseJunctionOutputR;
    std::vector<float> noseReflection;
    std::vector<float> noseDiameter;
    std::vector<float> noseA;
    std::vector<float> noseMaxAmplitude;

    float reflectionLeft, reflectionRight, reflectionNose;
    float newReflectionLeft, newReflectionRight, newReflectionNose;

    float constrictionIndex{ 3.0f };
    float constrictionDiameter{ 1.0f };
    float fricativeIntensity{ 0.0f };

    float lipOutput{};
    float noseOutput{};
};

} // namespace model
