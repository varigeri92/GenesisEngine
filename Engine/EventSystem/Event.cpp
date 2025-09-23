#include "gnspch.h"
#include "Event.h"

gns::EventListener::EventListener(std::function<void()> callback) :m_function(callback) {}


void gns::Event::Dispatch()
{
	for (auto& function : m_callbackStack)
	{
		function();
	}
}

void gns::Event::AddListener(EventListener& listener)
{
	m_callbackStack.push_back(listener.m_function);
	listener.m_fncId = m_callbackStack.size() - 1;
}

void gns::Event::RemoveListener(EventListener& listener)
{
	m_callbackStack.erase(m_callbackStack.begin(), m_callbackStack.begin() + listener.m_fncId);
}