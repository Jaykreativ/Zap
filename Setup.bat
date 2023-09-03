mkdir Dependencies
cd Dependencies
git clone https://github.com/Jaykreativ/Vulkan-Framework --recursive
git clone https://github.com/glfw/glfw --recursive
git clone https://github.com/g-truc/glm --recursive
::curl https://sdk.lunarg.com/sdk/download/1.3.261.0/windows/VulkanSDK-1.3.261.0-Installer.exe -O VulkanSDK-1.3.261.0-Installer.exe
::start /W VulkanSDK-1.3.261.0-Installer.exe -ArgumentList '/s'
::del VulkanSDK-1.3.261.0-Installer.exe
::echo Vulkan Dir: %VULKAN_SDK%
cd..
cmake .
cmake --build .
pause
