#include "model/Glottis.h"

namespace model {

Glottis::Glottis()
{
    reset();
}

void Glottis::reset()
{
    timeInWaveform = 0.0f;
    oldFrequency = targetFrequency;
    newFrequency = targetFrequency;
    smoothFrequency = targetFrequency;
    oldTenseness = targetTenseness;
    newTenseness = targetTenseness;
    totalTime = 0.0f;
    intensity = 0.0f;

    initWaveform();
}

void Glottis::prepareToPlay(float sampleRate)
{
    sampleRate_r = 1.0f / sampleRate;
}

float Glottis::tick(float lambda, float noise)
{
    timeInWaveform += sampleRate_r;
    totalTime += sampleRate_r;

    if (timeInWaveform > waveformLength)
    {
        timeInWaveform -= waveformLength;
        initWaveform (lambda);
    }

    float out = normalizedLFWaveform(timeInWaveform / waveformLength);
    float aspiration = intensity * (1.0f - sqrt(targetTenseness)) * getNoiseModulator() * noise;
    aspiration *= 0.2f + 0.02f * simplexNoise.sample1d(totalTime * 1.99f);
    out += aspiration;
    return out;
}

float Glottis::getNoiseModulator() const
{
    const float voiced{ 0.1f + 0.2f * fmax(0.0f, sin(MathConstants<float>::twoPi * timeInWaveform / waveformLength)) };
    return targetTenseness * intensity * voiced + (1.0f - targetTenseness * intensity) * 0.3f;
}

void Glottis::finishBlock()
{
    float vibrato = 0.0f;
    vibrato += vibratoAmount * sin(MathConstants<float>::twoPi * totalTime * vibratoFrequency);
    vibrato += 0.004f * simplexNoise.sample1d(totalTime * 4.07f);
    vibrato += 0.008f * simplexNoise.sample1d(totalTime * 2.15f);

    if (targetFrequency > smoothFrequency)
        smoothFrequency = fmin (smoothFrequency * 1.1f, targetFrequency);

    if (targetFrequency < smoothFrequency)
        smoothFrequency = fmax (smoothFrequency / 1.1f, targetFrequency);

    oldFrequency = newFrequency;
    newFrequency = smoothFrequency * (1.0f + vibrato);
    oldTenseness = newTenseness;
    newTenseness = targetTenseness
        + 0.1f * simplexNoise.sample1d(totalTime * 0.46f)
        + 0.05f * simplexNoise.sample1d(totalTime * 0.36f);

    if (! isTouched && alwaysVoice)
        newTenseness += (3.0f - targetTenseness) * (1.0f - intensity);

    if (isTouched || alwaysVoice)
        intensity += 0.13f;
    else
        intensity -= 0.05f;

    intensity = jlimit(0.0f, 1.0f, intensity);
}

void Glottis::setFrequency(float f, bool force)
{
    targetFrequency = f;

    if (force)
    {
        smoothFrequency = f;
        oldFrequency = f;
        newFrequency = f;

        initWaveform ();
    }
}

void Glottis::setTenseness(float t)
{
    targetTenseness = t;
}

void Glottis::initWaveform(float lambda)
{
    frequency = oldFrequency * (1.0f - lambda) + newFrequency * lambda;
    float tenseness = oldTenseness * (1.0f - lambda) + newTenseness * lambda;
    float Rd = jlimit(0.5f, 2.7f, 3.0f * (1.0f - tenseness));

    waveformLength = 1.0f / frequency;

    float Ra = -0.01f + 0.048f * Rd;
    float Rk = 0.224f + 0.118f * Rd;
    float Rg = (Rk / 4.0f) * (0.5f + 1.2f * Rk) / (0.11f * Rd - Ra * (0.5f + 1.2f * Rk));

    float Ta = Ra;
    float Tp = 1.0f / (2.0f * Rg);
    Te = Tp + Tp * Rk;

    epsilon = 1.0f / Ta;
    shift = exp (-epsilon * (1.0f - Te));
    delta = 1.0f - shift;

    float RHSIntegral = (1.0f / epsilon) * (shift - 1.0f) + (1.0f - Te) * shift;
    RHSIntegral = RHSIntegral / delta;

    float totalLowerIntegral = - (Te-Tp) / 2.0f + RHSIntegral;
    float totalUpperIntegral = -totalLowerIntegral;

    omega = MathConstants<float>::pi / Tp;
    float s = sin (omega * Te);

    float y = -MathConstants<float>::pi * s * totalUpperIntegral / (Tp * 2.0f);
    float z = log(y);
    alpha = z / (Tp / 2.0f - Te);
    E0 = -1.0f / (s * exp(alpha*Te));
}

float Glottis::normalizedLFWaveform (float t)
{
    float output = (t > Te) ? (-exp(-epsilon * (t - Te)) + shift) / delta
                            : E0 * exp(alpha * t) * sin(omega * t);

    return output * this->intensity * this->loudness;
}

} // namespace model
