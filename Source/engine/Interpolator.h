#pragma once

#include <JuceHeader.h>
#include <vector>

namespace engine {

/**
 * @brief Lagrange interpolator.
 *
 * @note This class does not apply an interpolation filter when downsampling.
 */
class Interpolator
{
public:
    Interpolator(float ratio = 1.0f, size_t nChannels = 1);

    void setRatio(float r) noexcept { ratio = r; }
    float getRatio() const noexcept { return ratio; }

    void setNumberOfChannels(size_t n);
    size_t getNumberOfChannels() const noexcept { return acc.size(); }

    void reset();

    bool canRead() const noexcept;
    bool readAllChannels(float* const x) noexcept;
    float readUnchecked(size_t channel) const noexcept;
    float readLinearUnchecked(size_t channel) const noexcept;
    void readIncrement();
    bool read(float& l, float& r) noexcept;


    bool canWrite() const noexcept;
    bool writeAllChannels(const float* const x) noexcept;
    void writeUnchecked(float x, size_t channel);
    void writeIncrement();
    bool write(float l, float r) noexcept;

private:
    using SamplesBuffer = std::array<float, 8>;

    std::vector<SamplesBuffer> acc;

    int accIndex{};
    float accFrac{};
    float ratio{};
};


} // namespace engine
