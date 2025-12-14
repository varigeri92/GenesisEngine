#pragma once

namespace gns
{
	class PathHelper
	{
		public:
		GNS_API static std::string AssetsPath;
		GNS_API static std::string ResourcesPath;

		GNS_API static std::string FromAssetsRelative(const std::string& relative_path);
		GNS_API static std::string FromResourcesRelative(const std::string& relative_path);
	};
}
