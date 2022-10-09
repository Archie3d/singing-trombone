#include "model/Noise.h"

namespace model {

WhiteNoise::WhiteNoise()
{
}

void WhiteNoise::setSeed(int64 seed)
{
    random.setSeed(seed);
}

float WhiteNoise::tick()
{
    return random.nextFloat();
}

} // namespace model
