@echo off

pushd build
copy ..\data\* . > NULL

cl /Zi /MDd /nologo /I ..\code\monarch_engine /I ..\external /Tc ..\code\build.c  opengl32.lib onecore.lib Gdi32.lib /Fe:main.exe
popd
