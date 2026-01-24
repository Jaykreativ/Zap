#pragma once

#include <vector>

namespace Zap {
	class Event {};

	template<class T>
	class EventHandler;

	template<class T>
	class EventListener {
	public:
		EventListener(EventHandler<T>& handler);
		~EventListener();

		virtual void callback(const T& event) = 0;

	private:
		EventHandler<T>* m_pHandler = nullptr;

		void resetHandler() {
			m_pHandler = nullptr;
		}

		friend EventHandler<T>;
	};

	template<class T>
	class EventHandler {
	private:
		std::vector<EventListener<T>*> m_listeners = {};

	public:
		void addListener(EventListener<T>* pListener) {
			m_listeners.push_back(pListener);
		}

		void removeListener(EventListener<T>* pListener) {
			for (auto it = m_listeners.begin(); it != m_listeners.end(); it++) {
				if (*it == pListener) {
					m_listeners.erase(it);
					return;
				}
			}
			ZP_WARN(false, "EventListenerList::remove(pListener) | listener is not part of list, cannot be removed");
		}

		void pushEvent(const T& event) const {
			for (EventListener<T>* pListener : m_listeners) {
				pListener->callback(event);
			}
		}

	protected:
		EventHandler() {
			static_assert(std::is_base_of_v<Event, T>, "EventHandler: Type has to be child class of Event");
		}
		~EventHandler() {
			for (auto* pListener : m_listeners) {
				pListener->resetHandler();
			}
		}
	};

	template<class T>
	EventListener<T>::EventListener(EventHandler<T>& handler)
		: m_pHandler(&handler)
	{
		static_assert(std::is_base_of_v<Event, T>, "EventListener: Type has to be child class of Event");
		m_pHandler->addListener(this);
	}

	template<class T>
	EventListener<T>::~EventListener() {
		if(m_pHandler)
			m_pHandler->removeListener(this);
	}

}

