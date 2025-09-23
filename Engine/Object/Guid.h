#pragma once
#include "../Utils/Random.h"

namespace gns
{
	typedef uint64_t guid;
	class Guid
	{
	public:
		static guid GetNewGuid()
		{
			return Random::Get<guid>();
		}
	};

	size_t hashString(const std::string& string);
}
