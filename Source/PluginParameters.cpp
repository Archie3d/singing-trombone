#include "PluginParameters.h"

namespace attr {

const static Identifier lyrics("lyrics");

} // namespace attr

PluginParameters::PluginParameters(AudioProcessor& audioProcessor)
    : processor{ audioProcessor }
{
}

void PluginParameters::serialize(OutputStream& os) const
{
    ReferenceCountedObjectPtr obj{ new DynamicObject() };
    obj->setProperty(attr::lyrics, lyrics);

    DBG("Serialize parameters:");
    DBG(JSON::toString(obj.get()));

    JSON::writeToStream(os, obj.get());
}

void PluginParameters::deserialize(InputStream& is)
{
    const auto state{ JSON::parse(is) };

    if (const auto* obj{ state.getDynamicObject()} ) {
        lyrics = obj->getProperty(attr::lyrics).toString();
    }
}
