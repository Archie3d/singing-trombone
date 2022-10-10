#include "model/Tract.h"

namespace model {

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

} // namespace model
