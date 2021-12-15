#pragma once
#include "v8/include/v8.h"

#include <string>

namespace BytecodeCompiler
{
    constexpr uint32_t currentVersion = 1;

    struct Bytecode
    {
        uint32_t version = 0;
        size_t size = 0;
        uint8_t* data = nullptr;

        bool IsValid()
        {
            return version == currentVersion && size != 0 && data != nullptr;
        }

        Bytecode(uint32_t version, size_t size, uint8_t* data) : version(version), size(size), data(data) {}
    };
}  // namespace BytecodeCompiler
