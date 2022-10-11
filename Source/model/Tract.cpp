#include "model/Tract.h"

namespace model {

static inline float moveTowards(float current, float target, float amount)
{
    if (current < target)
        return jmin(current + amount, target);
    
    return jmax(current - amount, target);
}

static inline float moveTowards(float current, float target, float amountUp, float amountDown)
{
    if (current < target)
        return jmin (current + amountUp, target);
    
    return jmax(current - amountDown, target);
}

//==============================================================================

Tract::Config::Config(int numSegments)
{
    if (numSegments != defaultNumSegments) {
        n = numSegments;
        const float k = float(n) / float (defaultNumSegments);
        bladeStart = (int)floor (bladeStart * k);
        tongueIndex = float(bladeStart);
        tipStart = (int)floor (tipStart * k);
        lipStart = (int)floor(lipStart * k);
        noseLength = (int)floor(noseLength * k);
        noseStart = n - noseLength + 1;
    }

    noseDiameter.resize(noseLength);
    tractDiameter.resize(n);

    jassert (n > 0);
    jassert (lipStart < n);
    jassert (tipStart < lipStart);
}

//==============================================================================

Tract::Tract()
{
}

void Tract::reset()
{
    jassert(config.n > 0);

    const size_t tractLength{ (size_t)config.n };
    diameter.resize(tractLength);
    restDiameter.resize(tractLength);
    targetDiameter.resize(tractLength);
    newDiameter.resize(tractLength);
    L.resize(tractLength);
    R.resize(tractLength);
    reflection.resize(tractLength + 1);
    newReflection.resize(tractLength + 1);
    junctionOutputL.resize(tractLength + 1);
    junctionOutputR.resize(tractLength + 1);
    A.resize(tractLength);
    maxAmplitude.resize(tractLength);
    
    jassert(config.noseLength > 0);
    jassert(config.noseLength < config.n);
    const size_t noseLength{ (size_t)config.noseLength };

    noseL.resize(noseLength);
    noseR.resize(noseLength);
    noseJunctionOutputL.resize(noseLength + 1);
    noseJunctionOutputR.resize(noseLength + 1);
    noseReflection.resize(noseLength);
    noseDiameter.resize(noseLength);
    noseA.resize(noseLength);
    noseMaxAmplitude.resize(noseLength);

    constrictionIndex = 3.0f * config.n / (float)Config::defaultNumSegments;

    std::fill(L.begin(), L.end(), 0.0f);
    std::fill(R.begin(), R.end(), 0.0f);
    std::fill(reflection.begin(), reflection.end(), 0.0f);
    std::fill(newReflection.begin(), newReflection.end(), 0.0f);
    std::fill(junctionOutputL.begin(), junctionOutputL.end(), 0.0f);
    std::fill(junctionOutputR.begin(), junctionOutputR.end(), 0.0f);
    std::fill(A.begin(), A.end(), 0.0f);
    std::fill(maxAmplitude.begin(), maxAmplitude.end(), 0.0f);

    std::fill(noseL.begin(), noseL.end(), 0.0f);
    std::fill(noseR.begin(), noseR.end(), 0.0f);
    std::fill(noseJunctionOutputL.begin(), noseJunctionOutputL.end(), 0.0f);
    std::fill(noseJunctionOutputR.begin(), noseJunctionOutputR.end(), 0.0f);
    std::fill(noseReflection.begin(), noseReflection.end(), 0.0f);
    std::fill(noseA.begin(), noseA.end(), 0.0f);
    std::fill(noseMaxAmplitude.begin(), noseMaxAmplitude.end(), 0.0f);

    initialize();
}

void Tract::prepareToPlay(float sr, float bt)
{
    sampleRate = sr;
    blockTime = bt;
    transientCount = 0;
}

void Tract::tick(float glottalOutput, float turbulenceNoise, float lambda, Glottis& glottis)
{
    // Mouth
    processTransients();
    addTurbulenceNoise(turbulenceNoise, glottis);

    //glottalReflection = -0.8 + 1.6 * Glottis.newTenseness;
    junctionOutputR[0] = L[0] * glottalReflection + glottalOutput;
    junctionOutputL[config.n] = R[config.n - 1] * lipReflection;

    for (int i = 1; i < config.n; ++i) {
        float r = reflection[i] * (1-lambda) + newReflection[i]*lambda;
        float w = r * (R[i - 1] + L[i]);
        junctionOutputR[i] = R[i - 1] - w;
        junctionOutputL[i] = L[i] + w;
    }

    // Nose junction
    {
        const int i{ config.noseStart };
        float r{ newReflectionLeft * (1.0f - lambda) + reflectionLeft * lambda };
        junctionOutputL[i] = r * this->R[i - 1] + (1.0f + r) * (noseL[0] + L[i]);
        r = newReflectionRight * (1.0f - lambda) + reflectionRight * lambda;
        junctionOutputR[i] = r * L[i] + (1 + r) * (R[i - 1] + noseL[0]);
        r = newReflectionNose * (1 - lambda) + reflectionNose * lambda;
        noseJunctionOutputR[0] = r * noseL[0] + (1.0f + r) * (L[i] + R[i - 1]);
    }

    const bool updateAmplitudes{ juce::Random::getSystemRandom().nextFloat() < 0.1f };

    for (int i = 0; i < config.n; ++i) {
        R[i] = junctionOutputR[i] * 0.999f;
        L[i] = junctionOutputL[i + 1] * 0.999f;

        //R[i] = Math.clamp(this->junctionOutputR[i] * this->fade, -1, 1);
        //L[i] = Math.clamp(this->junctionOutputL[i+1] * this->fade, -1, 1);

        if (updateAmplitudes) {
            const float amplitude{ fabs(R[i] + L[i]) };

            if (amplitude > maxAmplitude[i])
                maxAmplitude[i] = amplitude;
            else
                maxAmplitude[i] *= 0.999f;
        }
    }

    lipOutput = R[config.n - 1];

    // Nose
    noseJunctionOutputL[config.noseLength] = noseR[config.noseLength - 1] * lipReflection;

    for (int i=1; i < config.noseLength; ++i) {
        const float w{ noseReflection[i] * (noseR[i - 1] + noseL[i]) };
        noseJunctionOutputR[i] = noseR[i - 1] - w;
        noseJunctionOutputL[i] = noseL[i] + w;
    }

    for (int i=0; i < config.noseLength; i++) {
        noseR[i] = noseJunctionOutputR[i] * fade;
        noseL[i] = noseJunctionOutputL[i + 1] * fade;

        //noseR[i] = jlimit (-1.0f, 1.0f, noseJunctionOutputR[i] * fade);
        //noseL[i] = jlimit (-1.0f, 1.0f, noseJunctionOutputL[i + 1] * fade);

        if (updateAmplitudes) {
            const float amplitude{ fabs (noseR[i] + noseL[i]) };

            if (amplitude > noseMaxAmplitude[i])
                noseMaxAmplitude[i] = amplitude;
            else
                noseMaxAmplitude[i] *= 0.999f;
        }
    }

    noseOutput = noseR[config.noseLength - 1];
}

void Tract::finishBlock()
{
    reshapeTract (blockTime);
    calculateReflections();
    memcpy(config.tractDiameter.data(), diameter.data(), sizeof (float) * config.n);
    memcpy(config.noseDiameter.data(), noseDiameter.data(), sizeof (float) * config.noseLength);
}

void Tract::setRestDiameter(float tongueIndex, float tongueDiameter)
{
    config.tongueIndex = tongueIndex;
    config.tongueDiameter = tongueDiameter;

    for (int i = config.bladeStart; i < config.lipStart; ++i) {
        const float t{ 1.1f * MathConstants<float>::pi * (float)(tongueIndex - i) / (float)(config.tipStart - config.bladeStart) };
        const float fixedTongueDiameter{ 2.0f + (tongueDiameter - 2.0f) / 1.5f };
        float curve{ (1.5f - fixedTongueDiameter + 1.7f) * cos(t) };

        if (i == config.bladeStart - 2 || i == config.lipStart - 1)
            curve *= 0.8f;

        if (i == config.bladeStart || i == config.lipStart - 2)
            curve *= 0.94f;

        restDiameter[i] = 1.5f - curve;
    }

    for (int i = 0; i < config.n; ++i)
        targetDiameter[i] = restDiameter[i];
}

void Tract::setConstriction (float cindex, float cdiam, float fi)
{
    const float k{ float(config.n) / float(Config::defaultNumSegments) }; 

    constrictionIndex = cindex;
    constrictionDiameter = cdiam;
    fricativeIntensity = fi;

    // This is basically the Tract touch handling code
    velumTarget = 0.01f;

    if (constrictionIndex > config.noseStart && constrictionDiameter < -config.noseOffset)
        velumTarget = 0.4f;

    if (constrictionDiameter < -0.85f - config.noseOffset)
        return;

    float d = jmax (0.0f, constrictionDiameter - 0.3f);

    int width{ 2 };

    if (constrictionIndex < 25 * k)
        width = 10 * k;
    else if (constrictionIndex >= config.tipStart)
        width = 5 * k;
    else
        width = (10 - 5 * (constrictionIndex - 25 * k) / ((double)config.tipStart - 25 * k)) * k;

    if (constrictionIndex >= 2 && constrictionIndex < config.n && d < 3)
    {
        int intIndex = round(constrictionIndex);

        for (int i = -ceil (width) - 1; i < width + 1; ++i) {
            if (intIndex + i < 0 || intIndex + i >= config.n)
                continue;

            float relpos{ (intIndex + i) - constrictionIndex };
            relpos = abs(relpos) - 0.5f;
            
            float shrink{};

            if (relpos <= 0.0f)
                shrink = 0.0f;
            else if (relpos > width)
                shrink = 1.0f;
            else
                shrink = 0.5f * (1.0f - cos(MathConstants<float>::pi * relpos / (float) width));

            if (d < targetDiameter[intIndex + i])
                targetDiameter[intIndex + i] = d + (targetDiameter[intIndex + i] - d) * shrink;
        }
    }
}

int Tract::getTractIndexCount() const
{
    return config.n;
}

int Tract::getTongueIndexLowerBound() const
{
    return config.bladeStart + 2;
}

int Tract::getTongueIndexUpperBound() const
{
    return config.tipStart - 3;
}

void Tract::initialize()
{
    const float k{ float(config.n) / float(Config::defaultNumSegments) }; 

    for (int i = 0; i < config.n; ++i) {
        float d{ 0.0f };

        if (i < 7 * k - 0.5f)
            d = 0.6f;
        else if (i < 12 * k)
            d = 1.1f;
        else
            d = 1.5f;

        diameter[i] = restDiameter[i] = targetDiameter[i] = newDiameter[i] = d;
    }

    for (int i = 0; i < config.noseLength; ++i) {
        float d{ 2.0f * ((float) i / (float) config.noseLength) };
        if (d < 1.0f)
            d = 0.4f + 1.6f * d;
        else
            d = 0.5f + 1.5f * (2.0f - d);

        d = fmin (d, 1.9f);

        noseDiameter[i] = d;
    }

    newReflectionLeft = newReflectionRight = newReflectionNose = 0.0f;

    calculateReflections();
    calculateNoseReflections();
    noseDiameter[0] = velumTarget;

    memcpy(config.tractDiameter.data(), diameter.data(), sizeof (float) * config.n);
    memcpy(config.noseDiameter.data(), noseDiameter.data(), sizeof (float) * config.noseLength);
}

void Tract::calculateReflections()
{
    for (int i = 0; i < config.n; ++i)
        A[i] = diameter[i] * diameter[i];

    for (int i = 1; i < config.n; ++i) {
        reflection[i] = newReflection[i];

        newReflection[i] = (A[i] == 0.0f) ? 0.999f
                                          : (A[i - 1] - A[i]) / ( A[i - 1] + A[i]);

    }

    // At junction with nose
    reflectionLeft = newReflectionLeft;
    reflectionRight = newReflectionRight;
    reflectionNose = newReflectionNose;
    const float sum{ A[config.noseStart] + A[config.noseStart + 1] + noseA[0] };
    newReflectionLeft = (2.0f * A[config.noseStart] - sum) / sum;
    newReflectionRight = (2.0f * A[config.noseStart + 1] - sum) / sum;
    newReflectionNose = (2.0f * noseA[0] - sum) / sum;
}

void Tract::calculateNoseReflections()
{
    for (int i = 0; i < config.noseLength; ++i)
        noseA[i] = noseDiameter[i] * noseDiameter[i];

    for (int i = 1; i < config.noseLength; ++i)
        noseReflection[i] = (noseA[i - 1] - noseA[i]) / (noseA[i - 1] + noseA[i]);
}


void Tract::addTransient(int position)
{
    if (transientCount >= transients.size())
        return;

    for (auto& trans : transients) {
        if (!trans.living) {
            trans.position = position;
            trans.timeAlive = 0;
            trans.lifeTime = 0.2f;
            trans.strength = 0.3f;
            trans.exponent = 200;
            trans.living = true;

            ++transientCount;
            return;
        }
    }
}

void Tract::addTurbulenceNoise(float turbulenceNoise, Glottis& glottis)
{
    if (constrictionIndex < 2.0f || constrictionIndex >= (float)config.n - 2)
        return;

    if (constrictionDiameter <= 0.0f)
        return;

    float intensity = fricativeIntensity;
    addTurbulenceNoiseAtIndex(0.66f * turbulenceNoise * intensity, constrictionIndex, constrictionDiameter, glottis);
}

void Tract::addTurbulenceNoiseAtIndex(float turbulenceNoise, float index, float d, Glottis& glottis)
{
    const int i{ (int)floor(index) };

    const float delta{ index - (float)i };
    turbulenceNoise *= glottis.getNoiseModulator();
    const float thinness0{ jlimit (0.0f, 1.0f, 8.0f * (0.7f - d)) };
    const float openness{ jlimit (0.0f, 1.0f, 30.0f * (d - 0.3f)) };
    const float noise0{ turbulenceNoise * (1.0f - delta) * thinness0 * openness * 0.5f };
    const float noise1{ turbulenceNoise * delta * thinness0 * openness * 0.5f };
    R[i + 1] += noise0;
    L[i + 1] += noise0;
    R[i + 2] += noise1;
    L[i + 2] += noise1;
}

void Tract::reshapeTract(float deltaTime)
{
    float amount = deltaTime * movementSpeed;
    int newLastObstruction = -1;
    
    for (int i = 0; i < config.n; ++i) {
        const float d{ diameter[i] };
        const float td{ targetDiameter[i] };

        if (d <= 0.0f)
            newLastObstruction = i;

        float slowReturn{};

        if (i < config.noseStart)
            slowReturn = 0.6f;
        else if (i >= config.tipStart)
            slowReturn = 1.0f;
        else
            slowReturn = 0.6f + 0.4f * (i - config.noseStart) / (config.tipStart - config.noseStart);
        
        diameter[i] = moveTowards(d, td, slowReturn * amount, 2.0f * amount);
    }

    if (lastObstruction > -1 && newLastObstruction == -1 && noseA[0] < 0.05f)
        addTransient (lastObstruction);

    lastObstruction = newLastObstruction;

    amount = deltaTime * movementSpeed;
    noseDiameter[0] = moveTowards(noseDiameter[0], velumTarget, amount * 0.25f, amount * 0.1f);
    config.noseDiameter[0] = noseDiameter[0];
    noseA[0] = noseDiameter[0] * noseDiameter[0];
}

void Tract::processTransients()
{
    for (auto& trans : transients) {
        if (trans.living) {
            const float newTimeAlive{ trans.timeAlive + 1.0f / (sampleRate * 2.0f) };

            if (newTimeAlive > trans.lifeTime) {
                trans.living = false;
            } else {
                const float amplitude{ trans.strength * pow(2.0f, -trans.exponent * trans.timeAlive) };
                L[trans.position] += amplitude * 0.5f;
                R[trans.position] += amplitude * 0.5f;
            }

            trans.timeAlive = newTimeAlive;
        }
    }
}


} // namespace model
