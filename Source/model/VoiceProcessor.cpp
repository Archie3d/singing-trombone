#include "model/VoiceProcessor.h"

namespace model {

VoiceProcessor::VoiceProcessor()
{
}

void VoiceProcessor::prepareToPlay(float sampleRate, int samplesPerBlock)
{
    constexpr float q = 0.5f;
    constexpr float gain = 1.0f;
    constexpr float fricativeFreq = 1000.0f;
    constexpr float aspirateFreq = 500.0f;

    fricativeFilter.setCoefficients(juce::IIRCoefficients::makePeakFilter(sampleRate, fricativeFreq, q, gain));
    aspirateFilter.setCoefficients(juce::IIRCoefficients::makePeakFilter(sampleRate, aspirateFreq, q, gain));

    timePerBlock = float(samplesPerBlock) / sampleRate;

    glottis.prepareToPlay (sampleRate);
    tract.prepareToPlay (sampleRate, timePerBlock);
    whiteNoise.setSeed (Time::currentTimeMillis());
}

void VoiceProcessor::setControlPoint(const VoiceProcessor::ControlPoint& cp)
{
    targetControlPoint = cp;
}

void VoiceProcessor::trigger (const VoiceProcessor::ControlPoint& cp)
{
    setControlPoint(cp);

    tongueX = cp.tongueX;
    tongueY = cp.tongueY;
    constrictionX = cp.constrictionX;
    constrictionY = cp.constrictionY;

    glottis.reset();
    tract.reset();

    setTenseness(cp.tenseness);

    fricativeIntensity = 0.0f;

    glottis.setTouched(true);
    glottis.setVibrato(0.0f);

    update();
}

void VoiceProcessor::retrigger(const VoiceProcessor::ControlPoint& cp)
{
    setControlPoint(cp);

    //tongueX = cp.tongueX;
    //tongueY = cp.tongueY;
    //constrictionX = cp.constrictionX;
    //constrictionY = cp.constrictionY;

    //glottis.reset();
    //tract.reset();

    setTenseness(cp.tenseness);

    //fricativeIntensity = 0.0f;

    //update();
}

void VoiceProcessor::release()
{
    glottis.setTouched(false);
}

void VoiceProcessor::process(float* out, int numFrames)
{
    const float Nr{ 1.0f / float(numFrames) };

    for (int i = 0; i < numFrames; ++i) {
        const float pureNoise{ whiteNoise.tick() };
        const float asp{ aspirateFilter.processSingleSampleRaw(pureNoise) };
        const float fri{ fricativeFilter.processSingleSampleRaw (pureNoise) };

        // Glottis
        const float lambda1{ float(i) * Nr };
        const float lambda2{ (float(i) + 0.5f) * Nr };

        const float glot{ glottis.tick(lambda1, asp) };
        float vocalOutput{};
        tract.tick(glot, fri, lambda1, glottis);
        vocalOutput += tract.getLipOutput() + tract.getNoseOutput();
        tract.tick(glot, fri, lambda2, glottis);
        vocalOutput += tract.getLipOutput() + tract.getNoseOutput();
        vocalOutput *= 0.125f;

        out[i] = vocalOutput;
    }

    update();
}

void VoiceProcessor::setFrequency(float f, bool force)
{
    glottis.setFrequency(f, force);
}

void VoiceProcessor::setVibrato(float level)
{
    glottis.setVibrato(level);
}

void VoiceProcessor::setTenseness(float t)
{
    glottis.setTenseness(t);
}

void VoiceProcessor::update()
{
    const float tongueIndex{ tongueX * ((float)(tract.getTongueIndexUpperBound() - tract.getTongueIndexLowerBound())) + tract.getTongueIndexLowerBound() };
    const float innerTongueControlRadius{ 2.05f };
    const float outerTongueControlRadius{ 3.5f };
    const float tongueDiameter{ tongueY * (outerTongueControlRadius - innerTongueControlRadius) + innerTongueControlRadius };
    const float constrictionMin{ -2.0f };
    const float constrictionMax{ 2.0f };

    const float constrictionIndex{ constrictionX * (float)tract.getTractIndexCount() };
    float constrictionDiameter{ constrictionY * (constrictionMax - constrictionMin) + constrictionMin };

    if (constrictionIndex < 0.01f) {
        constrictionDiameter = constrictionMax;
    } else {
        fricativeIntensity += 0.1f;
        fricativeIntensity = jmin(1.0f, fricativeIntensity);
    }

    tract.setRestDiameter(tongueIndex, tongueDiameter);
    tract.setConstriction(constrictionIndex, constrictionDiameter, fricativeIntensity);
    glottis.finishBlock();
    tract.finishBlock();

    updateControlPoint();
}

void VoiceProcessor::updateControlPoint()
{
    tongueX = targetControlPoint.tongueX;
    tongueY = targetControlPoint.tongueY;
    constrictionX = targetControlPoint.constrictionX;
    constrictionY = targetControlPoint.constrictionY;
    setTenseness(targetControlPoint.tenseness);
}

} // namespace model
