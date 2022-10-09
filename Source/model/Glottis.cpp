#include "model/Glottis.h"

namespace model {

Glottis::Glottis()
{
    reset();
}

void Glottis::reset()
{
    timeInWaveform = 0.0f;

    initWaveform();
}

void Glottis::prepareToPlay(float sampleRate)
{
    sampleRate_r = 1.0f / sampleRate;
}

float Glottis::tick(float lambda, float noise)
{
    return 0.0f;
}

void Glottis::initWaveform(float lambda)
{
    
}

} // namespace model
