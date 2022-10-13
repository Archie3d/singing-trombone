#pragma once

#include <JuceHeader.h>

class PluginParameters
{
public:
    PluginParameters(AudioProcessor& audioProcessor);

    void serialize(OutputStream& os) const;
    void deserialize(InputStream& is);

    String lyrics{};

private:
    AudioProcessor& processor;
};