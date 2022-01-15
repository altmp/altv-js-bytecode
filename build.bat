@echo off

:: Build the project
cmake . -BBUILD
cmake --build BUILD --config Release
