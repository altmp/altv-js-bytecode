# Build the project
cmake . -BBUILD
cmake --build BUILD --config Release

# Copy output files
if [ ! -d "./dist" ]; then
    mkdir dist
fi
cp ./BUILD/Release/libjs-bytecode-module.so dist
