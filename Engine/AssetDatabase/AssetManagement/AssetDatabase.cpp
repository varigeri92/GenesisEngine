#include "gnspch.h"
#include "AssetDatabase.h"

std::function<gns::assets::AssetDatabase::Entry(gns::guid)> gns::assets::AssetDatabase::sGetDatabaseEntry = nullptr;

gns::assets::AssetDatabase::Entry gns::assets::AssetDatabase::GetDatabaseEntry(gns::guid asset_guid)
{
	if (sGetDatabaseEntry != nullptr)
		return sGetDatabaseEntry(asset_guid);
	else
		return {};
}

void gns::assets::AssetDatabase::SetGetterFunc(std::function<Entry(gns::guid)> getterFunc)
{
	sGetDatabaseEntry = getterFunc;
}
