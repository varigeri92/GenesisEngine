#pragma once
#include "../Object/Guid.h"

namespace gns::assets
{
	struct AssetInfo;
}

class IAssetInfoResolver
{
public:
	virtual ~IAssetInfoResolver() = default;
	virtual gns::assets::AssetInfo ResolveAssetInfo(gns::guid) = 0;
};
