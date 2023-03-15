@set SDL_DIR=E:\SDL\SDL2-devel-2.26.3-VS\SDL2-2.26.3
@set SDL2_INCLUDE=%SDL_DIR%\include
@set SDL2_LIBS=%SDL_DIR%\lib\x64

@set IMGUI_DIR=.\imgui
@set GLAD_DIR=.\glad
@set GLFW_DIR=.\glfw
@set GLM_DIR=.\glm
@set KHR_DIR=.\khr
@set STB_DIR=.\stb

@set INCLUDES=-I%IMGUI_DIR% -I%GLAD_DIR% -I%GLFW_DIR%\include -I%STB_DIR% -I%GLM_DIR% -I%KHR_DIR%
@set LIBS=-L%GLFW_DIR%\x86
@set LINKS=-lglfw3_mt -lglfw3dll -lglfw3 -lopengl32 -lShell32 

@set OUTPUT=TreeOfFlight.exe
@set OUTPUT_DIR=.\out

@set SRC=.\src
@set SOURCES=%SRC%\*.cpp %IMGUI_DIR%\*.cpp %GLAD_DIR%\glad\glad.c %GLM_DIR% 

mkdir %OUTPUT_DIR%

clang++ -std=c++20 %INCLUDES% %LIBS% %SOURCES% -o %OUTPUT_DIR%\%OUTPUT% %LINKS% -Xlinker /subsystem:windows