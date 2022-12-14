#pragma once

#include <JuceHeader.h>
#include <vector>

namespace engine {

class Lyrics
{
public:
    using Ptr = std::shared_ptr<Lyrics>;

    struct Phrase
    {
        String attack{};
        String release{};
        Range<int> position{};
    };

    Lyrics();
    virtual ~Lyrics() = default;
    void clear();
    Result parse0(const String& str);
    Result parse(const String& str);

    size_t size() const { return phrases.size(); }
    const Phrase& operator[](size_t index) const;

private:
    std::vector<Phrase> phrases{};
};

} // namespace engine
