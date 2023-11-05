#pragma once
#define GLM_FORCE_LEFT_HANDED
#include "VulkanFramework.h"
#include "PxPhysicsAPI.h"

//TODO add standart renderer for windows with no renderer
namespace Zap {
    class Window;

    class Base {
    public:
        void init();

        void terminate();

        static Base* createBase(const char* applicationName);

        static void releaseBase();

        static Base* getBase();

    private:
        Base(const char* applicationName);
        ~Base();

        bool m_isInit;

        const char* m_applicationName;

        //physx variables
        physx::PxFoundation* m_pxFoundation;
        physx::PxPhysics* m_pxPhysics;
        physx::PxScene* m_pxScene;

        static Base m_engineBase;
        static bool m_exists;
    };

    namespace objects {
        static std::vector<Window*> windows;
    }

    namespace GlobalSettings {
        VkFormat getColorFormat();
        VkFormat getDepthStencilFormat();
    }
}