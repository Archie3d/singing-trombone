#include "engine/Lyrics.h"

namespace engine {

template <typename T>
static bool isWhiteChar(const T t)
{
    return t == ' ' || t== ',' || t == '\t' || t == '\r' || t == '\n';
}

template <typename T>
static bool isAlphaOrDash(const T t)
{
    return (t >= 'a' && t <= 'z') || (t >= 'A' && t <= 'Z') || (t == '-');
}

//==============================================================================

Lyrics::Lyrics()
{
}

void Lyrics::clear()
{
    phrases.clear();
}

Result Lyrics::parse0(const String& str)
{
    clear();
    const StringArray tokens{ StringArray::fromTokens(str, " ,\t\r\n", "") };

    if (tokens.isEmpty())
        return Result::fail("Enpty lyrics");

    for (const auto& token : tokens) {
        const String sToken{ token.trim().toLowerCase() };

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

Result Lyrics::parse(const String& str)
{
    clear();

    const auto length{ str.length() };

    int pos{ 0 };

    auto isOver = [&]() { return pos >= length; };
    auto skipSpaces = [&]() { for (; pos < length && isWhiteChar(str[pos]); ++pos); };
    auto skipAlphaOrDash = [&]() { for (; pos < length && isAlphaOrDash(str[pos]); ++pos); };

    while (pos < length) {
        skipSpaces();
        if (isOver()) break;

        int startPos{ pos };
        skipAlphaOrDash();
        int stopPos{ pos };
        String s{ str.substring(startPos, stopPos) };

        DBG("   -> '" << s << "'");

        if (!s.isEmpty()) {
            Phrase phrase{};
            phrase.position = Range<int>(startPos, stopPos);

            const StringArray tokens{ StringArray::fromTokens(s, "-", "") };

            if (tokens.size() > 0)
                phrase.attack = tokens[0].trim().toLowerCase();
            if (tokens.size() > 1)
                phrase.release = tokens[1].trim();

            phrases.push_back(std::move(phrase));
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
