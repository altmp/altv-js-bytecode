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
        const uint8_t* data = nullptr;

        bool IsValid()
        {
            return version == currentVersion && size != 0 && data != nullptr;
        }

        Bytecode(uint32_t version, size_t size, const uint8_t* _data) : version(version), size(size), data(_data)
        {
            data = new uint8_t[size];
            memcpy((void*)data, _data, size);
        }
        ~Bytecode()
        {
            delete[] data;
        }
    };
}  // namespace BytecodeCompiler
