@echo off

:: Build the project
cmake . -BBUILD
cmake --build BUILD --config Release

:: Copy built binary to dist folder
IF NOT EXIST dist (
    mkdir dist
)
copy BUILD\Release\altv-js-bytecode.dll dist
copy BUILD\Release\altv-js-bytecode.pdb dist
