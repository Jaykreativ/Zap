cd %~dp0/..
%VULKAN_SDK%\Bin\glslangValidator.exe -V Shader\src\shader.vert -o .\shader.vert.spv
%VULKAN_SDK%\Bin\glslangValidator.exe -V Shader\src\shader.frag -o .\shader.frag.spv
%VULKAN_SDK%\Bin\glslangValidator.exe -V Shader\src\PBRShader.vert -o .\PBRShader.vert.spv
%VULKAN_SDK%\Bin\glslangValidator.exe -V Shader\src\PBRShader.frag -o .\PBRShader.frag.spv
pause