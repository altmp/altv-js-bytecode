#pragma once

#include "v8.h"
#include "cpp-sdk/SDK.h"

#include <vector>
#include <string>

namespace Compiler
{
    constexpr const char magicBytes[] = { 'A', 'L', 'T', 'B', 'C' };

    // Hash for empty module ("")
    constexpr uint32_t srcHash = 2147483648;
    constexpr int srcHashOffset = 8;

    constexpr uint32_t flagsHash = 1064582566;
    constexpr int flagsHashOffset = 12;

    // Compiles the module + all its dependencies to bytecode and writes it to the specified package
    void CompileModuleToBytecode(
      v8::Isolate* isolate, alt::IResource* resource, alt::IPackage* package, const std::string& fileName, const std::string& sourceCode, std::vector<std::string>& compiledFiles);
    void FixBytecode(const uint8_t* buffer);
    std::pair<uint8_t*, size_t> CreateBytecodeBuffer(const uint8_t* buffer, int length);
}  // namespace Compiler
