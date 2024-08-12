@echo off 
copy data\* build /Y > build\NULL
pushd build
main.exe
popd
