@echo off

REM -MTd statically link with c runtime library, the d means to use the debug version for debug purposes
REM should remove the d for release
REM add -P to compiler flags to get a .i file of your translation unit; NOTE that this doesn't actually compile
REM your code, so to actually update your EXE, you have to remove the flag before compiling again

set CommonCompilerFlags=-MTd -nologo -Gm- -GR- -EHa- -Oi -W4 -wd4201 -wd4100 -wd4189 -FC -Z7 -DGAME_INTERNAL=1 -DGAME_SLOW=1 -DGAME_WIN32=1
set CommonLinkerFlags=-incremental:no -opt:ref user32.lib gdi32.lib opengl32.lib glu32.lib winmm.lib Comdlg32.lib Shlwapi.lib

IF NOT EXIST ..\build mkdir ..\build
pushd ..\build
REM cl %CommonCompilerFlags% ..\src\game.cpp -Fmgame.map /LD /link -incremental:no opengl32.lib glu32.lib /EXPORT:GameGetSoundSamples /EXPORT:GameUpdateAndRender
cl %CommonCompilerFlags% ..\src\win32_game.cpp -Fmwin32_game.map /link %CommonLinkerFlags%
popd
