#pragma once
#include <functional>

struct Handle
{
	static constexpr size_t Invalid = static_cast<size_t>(-1);
};
struct TextureHandle
{
	size_t handle = Handle::Invalid;

	bool IsValid() const
	{
		return handle != Handle::Invalid;
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

struct MeshHandle
{
	size_t handle = Handle::Invalid;

	bool IsValid() const
	{
		return handle != Handle::Invalid;
	}

	inline bool operator==(const MeshHandle& other) const noexcept
	{
		return handle == other.handle;
	}

	inline bool operator!=(const MeshHandle& other) const noexcept
	{
		return handle != other.handle;
	}
};

struct ShaderHandle
{
	size_t handle = Handle::Invalid;

	bool IsValid() const
	{
		return handle != Handle::Invalid;
	}

	inline bool operator==(const ShaderHandle& other) const noexcept
	{
		return handle == other.handle;
	}

	inline bool operator!=(const ShaderHandle& other) const noexcept
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

	template<>
	struct hash<MeshHandle>
	{
		size_t operator()(const MeshHandle& h) const noexcept
		{
			return h.handle;
		}
	};

	template<>
	struct hash<ShaderHandle>
	{
		size_t operator()(const ShaderHandle& h) const noexcept
		{
			return h.handle;
		}
	};
}