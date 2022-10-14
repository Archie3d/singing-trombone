#pragma once

#include <JuceHeader.h>

class PluginParameters
{
public:
    PluginParameters(AudioProcessor& audioProcessor);

    void serialize(OutputStream& os) const;
    void deserialize(InputStream& is);

    String lyrics{};

    AudioParameterFloat* volume{};
    AudioParameterFloat* expression{};

    AudioParameterFloat* envelopeAttack{};
    AudioParameterFloat* envelopeDecay{};
    AudioParameterFloat* envelopeSustain{};
    AudioParameterFloat* envelopeRelease{};

    AudioParameterFloat* vibratoIntensity{};

    AudioParameterBool* legatoEnabled{};

private:
    AudioProcessor& processor;
};