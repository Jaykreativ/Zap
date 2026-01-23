#pragma once

#include "Zap/Zap.h"

#include "glm.hpp"

namespace Zap {
	namespace RenderEvent {
		class RenderEvent {};

		class Resize : public RenderEvent {
		public:
			glm::vec2 size;
		};
	}

	class RenderEventHandler;

	template<class T>
	class RenderEventListener {
	public:
		RenderEventListener(RenderEventHandler& handler);
		~RenderEventListener();

		virtual void callback(const T& event) = 0;
	private:
		RenderEventHandler& m_handler;
	};

	template<class T>
	class RenderEventListenerList {
	public:
		void add(RenderEventListener<T>* pListener) {
			m_listeners.push_back(pListener);
		}

		void remove(RenderEventListener<T>* pListener) {
			for (auto it = m_listeners.begin(); it != m_listeners.end(); it++) {
				if (*it == pListener) {
					m_listeners.erase(it);
					return;
				}
			}
			ZP_WARN(false, "RenderEventListenerList::remove(pListener) | listener is not part of list, cannot be removed");
		}

		void pushEvent(const T& event) const {
			for (RenderEventListener<T>* pListener : m_listeners) {
				pListener->callback(event);
			}
		}

	private:
		RenderEventListenerList() = default;
		~RenderEventListenerList() = default;

		std::vector<RenderEventListener<T>*> m_listeners = {};

		friend class RenderEventHandler;
	};

	class RenderEventHandler {
	public:
		/* T */
		//void addListener(RenderEventListener<RenderEvent::T>* listener) {
		//	m_TEventList.add(listener);
		//}
		//void removeListener(RenderEventListener<RenderEvent::T>* listener) {
		//	m_TEventList.remove(listener);
		//}
		//void pushEvent(const RenderEvent::T& event) const {
		//	m_TEventList.pushEvent(event);
		//}

		/* Resize */
		void addListener(RenderEventListener<RenderEvent::Resize>* listener) {
			m_ResizeEventList.add(listener);
		}
		void removeListener(RenderEventListener<RenderEvent::Resize>* listener) {
			m_ResizeEventList.remove(listener);
		}
		void pushEvent(const RenderEvent::Resize& event) const {
			m_ResizeEventList.pushEvent(event);
		}

	private:
		RenderEventHandler() = default;
		~RenderEventHandler() = default;

		//RenderEventListenerList<RenderEvent::T> m_TEventList;
		RenderEventListenerList<RenderEvent::Resize> m_ResizeEventList;

		friend class Renderer;
	};

	template<class T>
	RenderEventListener<T>::RenderEventListener(RenderEventHandler& handler)
		: m_handler(handler)
	{
		static_assert(std::is_base_of_v<RenderEvent::RenderEvent, T>, "Type has to be child class of RenderEvent");
		m_handler.addListener(this);
	}

	template<class T>
	RenderEventListener<T>::~RenderEventListener() {
		m_handler.removeListener(this);
	}
}