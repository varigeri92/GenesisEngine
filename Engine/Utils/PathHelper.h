#pragma once

namespace gns
{
	class PathHelper
	{
		public:
		static std::string AssetsPath;
		static std::string ResourcesPath;

		static std::string FromAssetsRelative(const std::string& relative_path);
		static std::string FromResourcesRelative(const std::string& relative_path);
	};
}
