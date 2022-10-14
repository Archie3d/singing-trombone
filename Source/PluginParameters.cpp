#include "PluginParameters.h"

namespace attr {

const static Identifier lyrics     ("lyrics");
const static Identifier volume     ("volume");
const static Identifier expression ("expression");
const static Identifier envelope   ("envelope");
const static Identifier attack     ("attack");
const static Identifier decay      ("decay");
const static Identifier sustain    ("sustain");
const static Identifier release    ("release");
const static Identifier vibrato    ("vibrato");
const static Identifier legato     ("legato");

} // namespace attr

PluginParameters::PluginParameters(AudioProcessor& audioProcessor)
    : processor{ audioProcessor }
{
    processor.addParameter(volume           = new AudioParameterFloat("volume",       "Volume",      0.0f, 1.0f, 1.0f  ));
    processor.addParameter(expression       = new AudioParameterFloat("expression",   "Expression",  0.0f, 1.0f, 1.0f  ));

    processor.addParameter(envelopeAttack   = new AudioParameterFloat("adsr_attack",  "Attack",      0.0f, 1.0f, 0.3f  ));
    processor.addParameter(envelopeDecay    = new AudioParameterFloat("adsr_decay",   "Decay",       0.0f, 1.0f, 0.1f  ));
    processor.addParameter(envelopeSustain  = new AudioParameterFloat("adsr_sustain", "Sustain",     0.0f, 1.0f, 0.75f ));
    processor.addParameter(envelopeRelease  = new AudioParameterFloat("adsr_release", "Release",     0.0f, 1.0f, 0.3f  ));

    processor.addParameter(vibratoIntensity = new AudioParameterFloat("vibrato",      "Vibrato",     0.0f, 1.0f, 1.0f  ));

    processor.addParameter(legatoEnabled    = new AudioParameterBool ("legato",       "Legato",      true));
}

void PluginParameters::serialize(OutputStream& os) const
{
    ReferenceCountedObjectPtr obj{ new DynamicObject() };
    obj->setProperty(attr::lyrics, lyrics);

    obj->setProperty(attr::volume, volume->get());
    obj->setProperty(attr::expression, volume->get());

    {
        ReferenceCountedObjectPtr env{ new DynamicObject() };
        env->setProperty(attr::attack,  envelopeAttack->get());
        env->setProperty(attr::decay,   envelopeDecay->get());
        env->setProperty(attr::sustain, envelopeSustain->get());
        env->setProperty(attr::release, envelopeRelease->get());

        obj->setProperty(attr::envelope, env.getObject());
    }

    obj->setProperty(attr::vibrato, vibratoIntensity->get());
    obj->setProperty(attr::legato, legatoEnabled->get());

    DBG("Serialize parameters:");
    DBG(JSON::toString(obj.get()));

    JSON::writeToStream(os, obj.get());
}

void PluginParameters::deserialize(InputStream& is)
{
    const auto state{ JSON::parse(is) };

    DBG(JSON::toString(state));

    if (const auto* obj{ state.getDynamicObject()} ) {
        lyrics = obj->getProperty(attr::lyrics).toString();

        if (auto v{ obj->getProperty(attr::volume)}; !v.isVoid())     volume->operator=((float)v);
        if (auto v{ obj->getProperty(attr::expression)}; !v.isVoid()) expression->operator=((float)v);

        if (const auto* env{ obj->getProperty(attr::envelope).getDynamicObject() }) {
            if (auto v{ env->getProperty(attr::attack)}; !v.isVoid())  envelopeAttack->operator=((float)v);
            if (auto v{ env->getProperty(attr::decay)}; !v.isVoid())   envelopeDecay->operator=((float)v);
            if (auto v{ env->getProperty(attr::sustain)}; !v.isVoid()) envelopeSustain->operator=((float)v);
            if (auto v{ env->getProperty(attr::release)}; !v.isVoid()) envelopeRelease->operator=((float)v);
        }

        if (auto v{ obj->getProperty(attr::vibrato)}; !v.isVoid()) vibratoIntensity->operator=((float)v);
        if (auto v{ obj->getProperty(attr::legato)}; !v.isVoid())  legatoEnabled->operator=((bool)v);
    }
}
