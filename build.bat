@REM Build for Visual Studio compiler. Run your copy of vcvars32.bat or vcvarsall.bat to setup command-line compiler.

@set SDL_DIR=E:\SDL\SDL2-devel-2.26.3-VS\SDL2-2.26.3
@set SDL2_INCLUDE=%SDL_DIR%\include
@set SDL2_LIBS=%SDL_DIR%\lib\x64

@set IMGUI_DIR=.\imgui
@set IMGUI_BACKEND=%IMGUI_DIR%\backends

@set INCLUDES=-I%SDL2_INCLUDE% -I%IMGUI_BACKEND% -I%IMGUI_DIR% 
@set LIBS=-L%SDL2_LIBS%
@set LINKS=-lSDL2main -lSDL2 -lShell32 

@set OUTPUT=shee.exe
@set OUTPUT_DIR=.

@set SRC=.\src
@set THROWAWAY=%SRC%\headers\tsu_math.cpp
@set SOURCES=%SRC%\main.cpp %IMGUI_BACKEND%\imgui_impl_sdl2.cpp %IMGUI_BACKEND%\imgui_impl_sdlrenderer.cpp %IMGUI_DIR%\*.cpp 

mkdir %OUTPUT_DIR%

clang %INCLUDES% %LIBS% %SOURCES% %THROWAWAY% -o %OUTPUT_DIR%\%OUTPUT% %LINKS% -Xlinker /subsystem:windows

@REM g++ -IE:\SDL\SDL2-devel-2.26.3-VS\SDL2-2.26.3\include -I.\imgui\backends -I.\imgui  -LE:\SDL\SDL2-devel-2.26.3-VS\SDL2-2.26.3\lib\x64 t.cpp .\imgui\backends\imgui_impl_sdl2.cpp .\imgui\backends\imgui_impl_sdlrenderer.cpp .\imgui\*.cpp  -o a\shee.exe -lSDL2main -lSDL2 -lShell32  -Xlinker /subsystem:windows