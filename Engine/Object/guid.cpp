#include "gnspch.h"
#include "Guid.h"

gns::guid gns::hashString(const std::string& string)
{
    std::hash<std::string> hasher;
    return hasher(string);
}

