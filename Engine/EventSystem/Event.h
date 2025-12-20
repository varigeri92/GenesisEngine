#pragma once
#include <functional>
#include <stack>

#include "../Object/Guid.h"


namespace gns
{
	template<typename... Args>
	struct EventListener_T 
	{
		bool operator==(const EventListener_T& other) const noexcept
		{
			return other.funcID == funcID;
		}

		EventListener_T(std::function<void(Args ... args)> func)
		{
			m_eventFunc = func;
			funcID = gns::Guid::GetNewGuid();
		}
		void Call(Args ... args) const
		{
			m_eventFunc(args ...);
		}

		void MarkDead()
		{
			dead = true;
		};
	private:
		std::function<void(Args ... args)> m_eventFunc;
		gns::guid funcID;
		bool dead;
	};

	template<typename... Args>
	class Event_T
	{
	private:
		std::vector<EventListener_T<Args ...>> m_listeners;
	public:
		void Dispatch(Args ... args)
		{
			for (const auto & listener : m_listeners)
			{
				listener.Call(args ...);
			}
		}

		void AddListener(EventListener_T<Args ...>& listener)
		{
			m_listeners.push_back(listener);
		}

		void RemoveListener(EventListener_T<Args ...>& listener)
		{
			listener.MarkDead();
		}

		void Clear()
		{
			m_listeners.clear();
		}
	};

	struct EventListener
	{
		friend class Event;
		GNS_API EventListener(std::function<void()> callback);
	private:
		std::function<void()> m_function;
		size_t m_fncId;
	};

	class Event
	{
	private:
		std::vector<std::function<void()>> m_callbackStack;

	public:
		GNS_API void Dispatch();
		GNS_API void AddListener(EventListener& listener);
		GNS_API void RemoveListener(EventListener& listener);
	};
}

