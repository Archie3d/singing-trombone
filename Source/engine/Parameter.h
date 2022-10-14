#pragma once

#include <JuceHeader.h>

namespace engine {

class Parameter
{
public:

    Parameter(float value = 0.0f, float min = 0.0f, float max = 1.0f, float smooth = 0.5f);
    
    void setName(const String& n) { name = n; }
    const juce::String& getName() const noexcept { return name; }
    void setValue(float v, float s, bool force = false);
    void setValue(float v, bool force = false);
    void setSmoothing(float s) noexcept;
    void setRange(float min, float max);

    Parameter& operator = (float v);

    float getCurrentValue() const noexcept { return currentValue; }
    float getTargetValue() const noexcept { return targetValue; }
    float getMin() const noexcept { return minValue; }
    float getMax() const noexcept { return maxValue; }
    bool isSmoothing() const noexcept { return smoothing || currentValue != targetValue; }

    float getNextValue();
    float getNextValue(size_t numFrames);

    float& targetRef() noexcept { return targetValue; }

private:

    void updateSmoothing();

    String name{};
    float currentValue{};
    float minValue{};
    float maxValue{ 1.0f };
    float targetValue{ 0.0f };
    float frac{};
    bool smoothing{};
};

//==============================================================================

class ParameterPool
{
public:
    ParameterPool(size_t size);
    size_t size() const { return params.size(); }
    Parameter& operator[] (size_t index);
    const Parameter& operator[] (size_t index) const;

    Parameter& findByName(const juce::String& n);

private:
    std::vector<Parameter> params{};
    Parameter dummyParameter{};
};

} // namespace engine
