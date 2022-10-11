#include "engine/Parameter.h"

namespace engine {

Parameter::Parameter(float value, float min, float max, float smooth)
    : currentValue{ value },
      minValue{ min },
      maxValue{ max },
      targetValue{ value },
      frac{ smooth },
      smoothing{ false }
{
}

void Parameter::setValue(float v, float s, bool force)
{
    targetValue = jlimit(minValue, maxValue, v);

    frac = jlimit(0.0f, 1.0f, s);

    if (force) {
        currentValue = targetValue;
        smoothing = false;
    } else {
        updateSmoothing();
    }
}

void Parameter::setValue(float v, bool force)
{
    targetValue = jlimit(minValue, maxValue, v);

    if (force) {
        currentValue = targetValue;
        smoothing = false;
    } else {
        updateSmoothing();
    }
}

void Parameter::setSmoothing(float s) noexcept
{
    frac = jlimit(0.0f, 1.0f, s);
}

void Parameter::setRange(float min, float max)
{
    minValue = jmin(min, max);
    maxValue = jmax(min, max);
}

Parameter& Parameter::operator = (float v)
{
    setValue(v);
    return *this;
}

float Parameter::getNextValue()
{
    updateSmoothing();

    if (smoothing) {
        const float prevValue{ currentValue };
        currentValue = targetValue * frac + currentValue * (1.0f - frac);

        // @todo Redundant statement
        if (fabsf(currentValue - prevValue) <= std::numeric_limits<float>::epsilon())
            currentValue = targetValue;
    }

    return currentValue;
}

void Parameter::updateSmoothing()
{
    smoothing = fabsf(currentValue - targetValue) > std::numeric_limits<float>::epsilon();

    if (!smoothing)
        currentValue = targetValue;
}

//==============================================================================

ParameterPool::ParameterPool(size_t size)
    : params(size)
{
}

Parameter& ParameterPool::operator[](size_t index)
{
    jassert(index >= 0 && index < params.size());

    if (index >= 0 && index < params.size())
        return params.at(index);

    return dummyParameter;
}

const Parameter& ParameterPool::operator[] (size_t index) const
{
    jassert(index >= 0 && index < params.size());

    if (index >= 0 && index < params.size())
        return params.at(index);

    return dummyParameter;
}

Parameter& ParameterPool::findByName(const String& n)
{
    for (auto& p : params) {
        if (p.getName() == n)
            return p;
    }

    return dummyParameter;
}

} // namespace engine
