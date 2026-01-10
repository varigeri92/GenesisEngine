#pragma once
#include "../Utils/Random.h"
#include "../Utils/Logger.h"
#include "../AssetDatabase/AssetTypes.h"

namespace gns
{
	typedef uint64_t guid;

	struct GnsHandle
	{
	private:
		guid m_guid		{0};
		assets::AssetType m_type		{assets::AssetType::None};
	public:
        gns::guid GetGuid() const { return m_guid; };
        assets::AssetType GetType() const { return m_type; };

        constexpr GnsHandle() noexcept = default;
        explicit constexpr GnsHandle(gns::guid guid) noexcept
            : m_guid(guid)
            , m_type(assets::AssetType::None)
        {
        }
		constexpr GnsHandle(gns::guid guid, assets::AssetType type_guid) noexcept
            : m_guid(guid)
            , m_type(type_guid)
        {
        }
		constexpr GnsHandle(const GnsHandle&) noexcept = default;
        constexpr GnsHandle(GnsHandle&&) noexcept = default;
        constexpr GnsHandle& operator=(const GnsHandle&) noexcept = default;
        constexpr GnsHandle& operator=(GnsHandle&&) noexcept = default;

        constexpr bool operator==(const GnsHandle& rhs) const noexcept
        {
            return m_guid == rhs.m_guid && m_type == rhs.m_type;
        }

        constexpr bool operator!=(const GnsHandle& rhs) const noexcept
        {
            return !(*this == rhs);
        }
		constexpr bool Equals(const GnsHandle& rhs) const noexcept
        {
            return *this == rhs;
        }

        constexpr bool IsNull() const noexcept { return (m_guid == 0); }
        explicit constexpr operator bool() const noexcept { return !IsNull(); }

		void AssignObject(guid new_guid, assets::AssetType type)
        {
            if (type == m_type)
				m_guid = new_guid;
            else
				LOG_ERROR("Type: " + std::to_string(static_cast<uint32_t>(type)) + " can't be assigned to "
				+ std::to_string(static_cast<uint32_t>(m_type)));
        }
	};

	class Guid
	{
		static GnsHandle s_gnsMullHandle;
	public:
		static GnsHandle GnsNullHandle() { return s_gnsMullHandle; }; const
		static guid GetNewGuid()
		{
			return Random::Get<guid>();
		}
	};

	size_t hashString(const std::string& string);
}
