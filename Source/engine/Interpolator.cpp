#include "engine/Interpolator.h"

namespace engine {

template <typename T>
T lerp (T a, T b, T frac) { return a + (b - a) * frac; }

template <typename T>
inline T lagr (T x_1, T x0, T x1, T x2, T frac) noexcept
{
    const T c1 = x1 - (1.0f / 3.0f) * x_1 - 0.5f * x0 - (1.0f / 6.0f) * x2;
    const T c2 = 0.5f * (x_1 + x1) - x0;
    const T c3 = (1.0f / 6.0f) * (x2 - x_1) + 0.5f * (x0 - x1);
    return ((c3 * frac + c2) * frac + c1) * frac + x0;
}

template <typename T>
inline T lagr (const T* const x, T frac) noexcept
{
    const T c1 = x[2] - (1.0f / 3.0f) * x[0] - 0.5f * x[1] - (1.0f / 6.0f) * x[3];
    const T c2 = 0.5f * (x[0] + x[2]) - x[1];
    const T c3 = (1.0f / 6.0f) * (x[3] - x[0]) + 0.5f * (x[1] - x[2]);
    return ((c3 * frac + c2) * frac + c1) * frac + x[1];
}

//==============================================================================

Interpolator::Interpolator(float ratio, size_t nChannels)
    : acc(nChannels)
    , accIndex{0}
    , accFrac{0.0f}
    , ratio{ratio}
{
    jassert(nChannels > 0);
}

void Interpolator::setNumberOfChannels(size_t n)
{
    jassert(n > 0);
    acc.resize(n);
    reset();
}

void Interpolator::reset()
{
    for (auto& buf : acc) {
        ::memset(buf.data(), 0, sizeof(float) * 8);
    }

    accIndex = 0;
    accFrac = 0.0f;
}

bool Interpolator::canRead() const noexcept
{
    return accFrac < 1.0f;
}

bool Interpolator::readAllChannels(float* const x) noexcept
{
    jassert(x != nullptr);

    if (accFrac >= 1.0f)
        return false;

    for (size_t i = 0; i < acc.size(); ++i) {
        x[i] = lagr(&acc[i].data()[accIndex], accFrac);
    }

    accFrac += ratio;

    return true;
}

float Interpolator::readUnchecked(size_t channel) const noexcept
{
    jassert(accFrac < 1.0f);

    return lagr(&acc[channel].data()[accIndex], accFrac);
}

float Interpolator::readLinearUnchecked(size_t channel) const noexcept
{
    jassert(accFrac < 1.0f);

    return lerp(acc[channel].data()[accIndex], acc[channel].data()[accIndex + 1], accFrac);
}

void Interpolator::readIncrement()
{
    accFrac += ratio;
}

bool Interpolator::read(float& l, float& r) noexcept
{
    jassert(acc.size() > 1);

    if (accFrac >= 1.0f)
        return false;

    l = lagr(&acc[0].data()[accIndex], accFrac);
    r = lagr(&acc[1].data()[accIndex], accFrac);

    accFrac += ratio;

    return true;
}

bool Interpolator::canWrite() const noexcept
{
    return accFrac >= 1.0f;
}

bool Interpolator::writeAllChannels(const float* const x) noexcept
{
    jassert(x != nullptr);

    if (accFrac < 1.0f)
        return false;

    for (size_t i = 0; i < acc.size(); ++i) {
        acc[i][accIndex] = acc[i][accIndex + 4] = x[i];
    }

    accIndex = (accIndex + 1) % 4;
    accFrac -= 1.0f;

    return true;
}

void Interpolator::writeUnchecked(float x, size_t channel)
{
    jassert(acc.size() > 1);

    acc[channel][accIndex] = acc[channel][accIndex + 4] = x;
}

void Interpolator::writeIncrement()
{
    accIndex = (accIndex + 1) % 4;
    accFrac -= 1.0f;
}

bool Interpolator::write(float l, float r) noexcept
{
    jassert(acc.size() > 1);

    if (accFrac < 1.0f)
        return false;

    acc[0][accIndex] = acc[0][accIndex + 4] = l;
    acc[1][accIndex] = acc[1][accIndex + 4] = r;

    accIndex = (accIndex + 1) % 4;
    accFrac -= 1.0f;

    return true;
}

} // namespace engine
