#pragma once
#include "v8/include/v8.h"

#include <string>

struct Bytecode
{
    static constexpr uint32_t currentVersion = 1;

    uint32_t version = 0;
    size_t size = 0;
    const uint8_t* data = nullptr;

    bool IsValid()
    {
        return version == currentVersion && size != 0 && data != nullptr;
    }

    Bytecode() {}
    Bytecode(size_t size, const uint8_t* _data) : version(currentVersion), size(size), data(_data)
    {
        data = new uint8_t[size];
        memcpy((void*)data, _data, size);
    }
    ~Bytecode()
    {
        delete[] data;
    }

    std::pair<uint8_t*, size_t> ToBuffer()
    {
        size_t bufferSize = sizeof(Bytecode) - sizeof(data) + size;
        uint8_t* buffer = new uint8_t[bufferSize];
        memcpy(buffer, &version, sizeof(version));
        memcpy(buffer + sizeof(version), &size, sizeof(size));
        size_t offset = sizeof(version) + sizeof(size);
        for(size_t i = 0; i < size; ++i)
        {
            buffer[offset + i] = data[i];
        }
        return std::make_pair(buffer, bufferSize);
    }

    static Bytecode FromBuffer(const uint8_t* buffer, size_t size)
    {
        Bytecode bytecode;
        if(size < sizeof(bytecode)) return bytecode;
        memcpy(&bytecode.version, buffer, sizeof(bytecode.version));
        memcpy(&bytecode.size, buffer + sizeof(bytecode.version), sizeof(bytecode.size));
        bytecode.data = new uint8_t[bytecode.size];
        memcpy((void*)bytecode.data, buffer + sizeof(bytecode.version) + sizeof(bytecode.size), bytecode.size);
        return bytecode;
    }
};
