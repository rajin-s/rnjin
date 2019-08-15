set shc=C:\VulkanSDK\1.1.106.0\Bin\glslangValidator.exe

%shc% -V .\hello_triangle.vert -o ..\..\build\shaders\hello_triangle.vert.spv > glsl.log
%shc% -V .\hello_triangle.frag -o ..\..\build\shaders\hello_triangle.frag.spv >> glsl.log