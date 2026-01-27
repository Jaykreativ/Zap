#pragma once

#include "glm.hpp"

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
		if (m_pHandler)
			m_pHandler->removeListener(this);
	}

	namespace RenderEvent {
		class RenderEvent : public Event {};

		class Resize : public RenderEvent {
		public:
			glm::vec2 size;
		};
	}

	class RenderEventHandler :
		public EventHandler<RenderEvent::Resize>
	{
		friend class Renderer;
	};

	class Actor; // forward declaration
	class Scene;
	namespace SceneEvent {
		class SceneEvent : public Event {};

		class Update : public SceneEvent {
		public:
			Update(Scene* pScene)
				: pScene(pScene)
			{}
			Scene* pScene = nullptr;
		};

		class AddActor : public SceneEvent {
		public:
			AddActor(Scene* pScene, Actor& actor)
				: pScene(pScene), actor(actor)
			{}
			Scene* pScene;
			Actor& actor;
		};

		class AddLight : public AddActor {
		public:
			AddLight(Scene* pScene, Actor& actor, uint32_t lightCount)
				: AddActor(pScene, actor), lightCount(lightCount)
			{}
			uint32_t lightCount = 0;
		};

		class AddModel : public AddActor {
		public:
			AddModel(Scene* pScene, Actor& actor, uint32_t modelCount)
				: AddActor(pScene, actor), modelCount(modelCount)
			{}
			uint32_t modelCount = 0;
		};

		class RemoveActor : public SceneEvent {
		public:
			RemoveActor(Scene* pScene, Actor& actor)
				: pScene(pScene), actor(actor)
			{}
			Scene* pScene;
			Actor& actor;
		};

		class RemoveLight : public RemoveActor {
		public:
			RemoveLight(Scene* pScene, Actor& actor, uint32_t lightCount)
				: RemoveActor(pScene, actor), lightCount(lightCount)
			{}
			uint32_t lightCount = 0;
		};

		class RemoveModel : public RemoveActor {
		public:
			RemoveModel(Scene* pScene, Actor& actor, uint32_t modelCount)
				: RemoveActor(pScene, actor), modelCount(modelCount)
			{}
			uint32_t modelCount = 0;
		};
	}

	class SceneEventHandler :
		public EventHandler<SceneEvent::Update>,
		public EventHandler<SceneEvent::AddActor>,
		public EventHandler<SceneEvent::AddLight>,
		public EventHandler<SceneEvent::AddModel>,
		public EventHandler<SceneEvent::RemoveActor>,
		public EventHandler<SceneEvent::RemoveLight>,
		public EventHandler<SceneEvent::RemoveModel>
	{
		friend class Scene;
	};

	class Window;
	namespace WindowEvent {
		class WindowEvent : public Event {};

		class Resize : public WindowEvent {
		public:
			Resize(Window* pWindow, int width, int height)
				: pWindow(pWindow), width(width), height(height)
			{};

			Window* pWindow = nullptr;
			int width = 0;
			int height = 0;
		};

		class Key : public WindowEvent {
		public:
			Key(Window* pWindow, int key, int scancode, int action, int mods)
				: pWindow(pWindow), key(key), scancode(scancode), action(action), mods(mods)
			{}

			Window* pWindow = nullptr;
			int key = 0;
			int scancode = 0;
			int action = 0;
			int mods = 0;
		};

		class CursorPos : public WindowEvent {
		public:
			CursorPos(Window* pWindow, double xPos = 0, double yPos = 0)
				: pWindow(pWindow), xPos(xPos), yPos(yPos)
			{}

			Window* pWindow = nullptr;
			double xPos = 0;
			double yPos = 0;
		};

		class MouseButton : public WindowEvent {
		public:
			MouseButton(Window* pWindow, int button, int action, int mods)
				: pWindow(pWindow), button(button), action(action), mods(mods)
			{}

			//TODO add Zap intern System for button indexing
			Window* pWindow = nullptr;
			int button = 0;
			int action = 0;
			int mods = 0;
		};

		class Scroll : public WindowEvent {
		public:
			Scroll(Window* pWindow, double xoffset, double yoffset)
				: pWindow(pWindow), xoffset(xoffset), yoffset(yoffset)
			{}

			Window* pWindow = nullptr;
			double xoffset = 0;
			double yoffset = 0;
		};

		class DragDrop : public WindowEvent {
		public:
			DragDrop(Window* pWindow, int pathCount, const char** paths)
				: pWindow(pWindow), pathCount(pathCount), paths(paths)
			{}

			Window* pWindow = nullptr;
			int pathCount;
			const char** paths;
		};
	}

	class WindowEventHandler :
		public EventHandler<WindowEvent::Resize>,
		public EventHandler<WindowEvent::Key>,
		public EventHandler<WindowEvent::CursorPos>,
		public EventHandler<WindowEvent::MouseButton>,
		public EventHandler<WindowEvent::Scroll>,
		public EventHandler<WindowEvent::DragDrop>
	{
		friend class Window;
	};

	class Texture;
	namespace AssetHandlerEvent {
		class AssetHandlerEvent : public Event {};

		class TextureLoad : public AssetHandlerEvent {
		public:
			TextureLoad(Texture& texture)
				: texture(texture)
			{}

			Texture& texture;
		};
	}

	class AssetHandlerEventHandler :
		public EventHandler<AssetHandlerEvent::TextureLoad>
	{
		friend class AssetHandler;
	};
}

