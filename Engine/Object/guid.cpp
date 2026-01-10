#include "gnspch.h"
#include "Guid.h"

gns::GnsHandle gns::Guid::s_gnsMullHandle = {0, assets::AssetType::None};
gns::guid gns::hashString(const std::string& string)
{
    std::hash<std::string> hasher;
    return hasher(string);
}

