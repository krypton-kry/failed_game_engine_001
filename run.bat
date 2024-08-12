@echo off
pushd build
start /wait main.exe
echo Exited with code : %errorlevel%
popd
