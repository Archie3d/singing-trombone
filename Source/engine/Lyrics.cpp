#include "engine/Lyrics.h"

namespace engine {

Lyrics::Lyrics()
{
}

void Lyrics::clear()
{
    phrases.clear();
}

Result Lyrics::parse(const String& str)
{
    clear();
    const StringArray tokens{ StringArray::fromTokens(str, " ,\t\r\n", "") };

    if (tokens.isEmpty())
        return Result::fail("Enpty lyrics");

    for (const auto& token : tokens) {
        const String sToken{ token.trim() };

        if (!sToken.isEmpty()) {
            const StringArray sPhrase{ StringArray::fromTokens(sToken, "-", "") };

            if (sPhrase.isEmpty())
                return Result::fail("Empty phrase detected");

            if (sPhrase.size() > 2)
                return Result::fail("Too many parts in a phrase");

            if (sPhrase.size() > 0) {
                Phrase phrase{};
                phrase.attack = sPhrase[0].trim();

                if (phrase.attack.isEmpty())
                    return Result::fail("Attack part of the phrase is missing");

                if (sPhrase.size() > 1) {
                    phrase.release = sPhrase[1].trim();
                }

                phrases.push_back(std::move(phrase));
            }
        }
    }

    return Result::ok();
}

const Lyrics::Phrase& Lyrics::operator[](size_t index) const
{
    const static Phrase dummy{};

    if (index < phrases.size())
        return phrases.at(index);

    return dummy;
}

} // namespace engine
