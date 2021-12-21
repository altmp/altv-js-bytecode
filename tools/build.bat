@echo off

:: Build the project
cmake . -BBUILD
cmake --build BUILD --config Release

:: Copy built binary to dist folder
IF NOT EXIST dist (
    mkdir dist
)
copy BUILD\Release\js-bytecode-module.dll dist
copy BUILD\Release\js-bytecode-module.pdb dist
