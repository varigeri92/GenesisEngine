#pragma once
#include <functional>
#include "../../Object/Guid.h"

namespace gns::assets
{
	class AssetDatabase
	{
	public:
		struct Entry
		{
			bool valid{ false };
			std::string filepath {""};
			size_t offset {(size_t)-1};
			size_t size{ (size_t)-1 };
		};

		GNS_API static Entry GetDatabaseEntry(gns::guid asset_guid);
		GNS_API  static void SetGetterFunc(std::function<Entry(gns::guid)> getterFunc);
	private:
		GNS_API static std::function<Entry(gns::guid)> sGetDatabaseEntry;
	};
}
