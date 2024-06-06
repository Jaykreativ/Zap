#pragma once

#include <set>

namespace Zap {
	template<typename T>
	class EventHandler
	{
	private:
		// uses a string of the combined values of the function ptr and the data pointer as key
		std::set<std::pair<void (*)(T& eventParams, void* customParams), void*>> m_callbacks = {};

		//std::vector<void*> m_customParams = {};
		//std::vector<void (*)(T& eventParams, void* customParams)> m_callbacks = {};

	public:
		EventHandler() = default;
		~EventHandler() = default;

		void pushEvent(T& event) {
			for (auto const& callbackPair : m_callbacks) {
				callbackPair.first(event, callbackPair.second);
			}
		}

		void addCallback(void (*callback)(T& eventParams, void* customParams), void* customData = nullptr) {
			m_callbacks.insert(std::pair<void (*)(T & eventParams, void* customParams), void*>(callback, customData));
		}

		void removeCallback(void (*callback)(T& eventParams, void* customParams), void* customData = nullptr) {
			m_callbacks.erase(std::pair<void (*)(T & eventParams, void* customParams), void*>(callback, customData));
		}
	};
}

