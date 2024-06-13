git clone https://github.com/Jaykreativ/VulkanFramework --recursive
git clone https://github.com/glfw/glfw --recursive
git clone -b docking https://github.com/ocornut/imgui --recursive
git clone https://github.com/g-truc/glm --recursive
git clone https://github.com/assimp/assimp --recursive
git clone https://github.com/NVIDIA-Omniverse/PhysX --recursive
::curl https://sdk.lunarg.com/sdk/download/1.3.261.0/windows/VulkanSDK-1.3.261.0-Installer.exe -O VulkanSDK-1.3.261.0-Installer.exe
::start /W VulkanSDK-1.3.261.0-Installer.exe -ArgumentList '/s'
::del VulkanSDK-1.3.261.0-Installer.exe
::echo Vulkan Dir: %VULKAN_SDK%
pause
