#pragma once
#include <functional>

struct TextureHandle
{
	static constexpr size_t Invalid = static_cast<size_t>(-1);
	size_t handle = Invalid;

	bool IsValid() const
	{
		return handle != Invalid;
	}

	inline bool operator==(const TextureHandle& other) const noexcept
	{
		return handle == other.handle;
	}

	inline bool operator!=(const TextureHandle& other) const noexcept
	{
		return handle != other.handle;
	}
};

namespace std
{
	template<>
	struct hash<TextureHandle>
	{
		size_t operator()(const TextureHandle& h) const noexcept
		{
			return h.handle;
		}
	};
}