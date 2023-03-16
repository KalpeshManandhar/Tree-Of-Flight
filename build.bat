setlocal

set IMGUI_DIR=.\imgui
set GLAD_DIR=.\glad
set GLFW_DIR=.\glfw
set GLM_DIR=.\glm
set KHR_DIR=.\khr
set STB_DIR=.\stb

set INCLUDES=-I%IMGUI_DIR% -I%GLAD_DIR% -I%GLFW_DIR%\include -I%STB_DIR% -I%GLM_DIR% -I%KHR_DIR%
set LIBS=-L%GLFW_DIR%\x64
set LINKS=-lopengl32 -lglfw3dll -lglfw3 -lglfw3_mt -lShell32 

set OUTPUT=TreeOfFlight.exe
set OUTPUT_DIR=.\out

set SRC=.\src
set SOURCES=%SRC%\*.cpp %IMGUI_DIR%\*.cpp %GLAD_DIR%\glad\glad.c

mkdir %OUTPUT_DIR%

clang++ -std=c++20 -DNDEBUG %INCLUDES% %LIBS% %SOURCES% -o %OUTPUT_DIR%\%OUTPUT% %LINKS% -Xlinker /subsystem:windows

copy %GLFW_DIR%\x64\glfw3.dll %OUTPUT_DIR%\