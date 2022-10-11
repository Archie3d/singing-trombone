#include "engine/Envelope.h"
#include <cmath>

namespace engine {

Envelope::Envelope()
{
}

void Envelope::trigger(const Envelope::Spec& spec)
{
    sustainLevel = spec.sustain;
    sampleRate = spec.sampleRate;

    attackRate = spec.attack * sampleRate;
    attackCoef = calculate(attackRate, attackTargetRatio);
    attackBase = (1.0f + attackTargetRatio) * (1.0f - attackCoef);

    decayRate = spec.decay * sampleRate;
    decayCoef = calculate(decayRate, decayReleaseTargetRatio);
    decayBase = (sustainLevel - decayReleaseTargetRatio) * (1.0f - decayCoef);

    releaseRate = spec.release * sampleRate;
    releaseCoef = calculate(releaseRate, decayReleaseTargetRatio);
    releaseBase = -decayReleaseTargetRatio * (1.0f - releaseCoef);

    currentState = State::Attack;
    currentLevel = 0.0f;
}

void Envelope::release()
{
    currentState = State::Release;
}

void Envelope::release(float t)
{
    releaseRate = t * sampleRate;
    releaseCoef = calculate(releaseRate, decayReleaseTargetRatio);
    releaseBase = -decayReleaseTargetRatio * (1.0f - releaseCoef);

    currentState = State::Release;
}

float Envelope::getNext()
{
    switch (currentState) {
    case State::Off:
        break;
    case State::Attack:
        currentLevel = attackBase + currentLevel * attackCoef;

        if (currentLevel >= 1.0f) {
            currentLevel = 1.0f;
            currentState = State::Decay;
        }
        break;
    case State::Decay:
        currentLevel = decayBase + currentLevel * decayCoef;

        if (currentLevel <= sustainLevel) {
            currentLevel = sustainLevel;
            currentState = State::Sustain;
        }
        break;
    case State::Sustain:
        break;
    case State::Release:
        currentLevel = releaseBase + currentLevel * releaseCoef;

        if (currentLevel <= 0.0f) {
            currentLevel = 0.0f;
            currentState = State::Off;
        }
        break;
    default:
        break;
    }

    return currentLevel;
}

float Envelope::calculate(float rate, float targetRatio)
{
    return rate <= 0 ? 0.0f : std::exp(-std::log((1.0f + targetRatio) / targetRatio) / rate);
}


} // namespace engine
