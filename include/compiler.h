#pragma once
#include "v8.h"

namespace BytecodeCompiler 
{
    extern uint32_t GetVersion();

    struct Bytecode
    {
        uint8_t* bytecode;
        int size;
        uint32_t version;

        Bytecode(v8::ScriptCompiler::CachedData* data) : size(data->length), version(BytecodeCompiler::GetVersion()) 
        {
            bytecode = new uint8_t[data->length];
            memcpy(bytecode, data->data, data->length);
        }
        Bytecode() : bytecode(nullptr), size(0), version(0) {}
        ~Bytecode() { delete bytecode; }
    };

    extern Bytecode CompileSourceIntoBytecode(v8::Isolate* isolate, v8::ScriptOrigin& origin, const char* sourceCode);
}
