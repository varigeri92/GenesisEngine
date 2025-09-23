#pragma once
#include <functional>
#include <stack>

namespace gns
{
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

