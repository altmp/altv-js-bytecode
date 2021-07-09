#pragma once
#include "v8.h"

namespace BytecodeCompiler 
{
    inline uint32_t GetVersion() 
    {
        return v8::ScriptCompiler::CachedDataVersionTag();
    }

    struct Bytecode
    {
        uint32_t version;
        int size;
        uint8_t* bytecode;

        Bytecode(v8::ScriptCompiler::CachedData* data) : size(data->length), version(BytecodeCompiler::GetVersion()) 
        {
            bytecode = new uint8_t[data->length];
            memcpy(bytecode, data->data, data->length);
        }
        Bytecode() : bytecode(nullptr), size(0), version(0) {}
        ~Bytecode() { delete bytecode; }
    };
}
